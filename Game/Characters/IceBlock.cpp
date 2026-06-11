#include "IceBlock.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "GridObjectComponent.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "GameTime.h"
#include "SnoBeeCharacter.h"
#include <cmath>

namespace dae
{
    IceBlock::IceBlock(ResourceManager& resourceManager)
        : GameObject("IceBlock")
    {
        m_pRenderComponent = AddComponent<RenderComponent>(resourceManager);
        m_pRenderComponent->SetTexture("iceblock.png");
        m_pRenderComponent->SetRenderSize(32.0f, 32.0f);
        
        m_miscTexture = resourceManager.LoadTexture("misc.png");
        m_scoresTexture = resourceManager.LoadTexture("scores.png");

        AddComponent<GridObjectComponent>();

        SetActive(false);
    }

    void IceBlock::SetActive(bool active)
    {
        m_isActive = active;
        if (auto pGridComp = GetComponent<GridObjectComponent>())
        {
            if (active)
            {
                pGridComp->Enable();
            }
            else
            {
                pGridComp->Disable();
            }
        }
    }

    void IceBlock::Render() const
    {
        if (m_isActive)
        {
            if (m_state == State::Crushing)
            {
                if (m_miscTexture)
                {
                    const auto& worldPos = GetWorldPosition();
                    Rect animSrc = { m_crushFrame * 16.0f, 48.0f, 16.0f, 16.0f };
                    Renderer::GetInstance().RenderTexture(*m_miscTexture, animSrc, worldPos.x, worldPos.y, 32.0f, 32.0f);
                }
            }
            else if (m_state == State::ShowingScore)
            {
                if (m_scoresTexture)
                {
                    // Blue 400 (second row, second column) in scores.png, drawn centred on the tile
                    const auto& worldPos = GetWorldPosition();
                    const Rect scoreSrc = { 16.0f, 16.0f, 16.0f, 16.0f };
                    Renderer::GetInstance().RenderTexture(*m_scoresTexture, scoreSrc, worldPos.x , worldPos.y, 32.0f, 32.0f);
                }
            }
            else
            {
                GameObject::Render();
            }
        }
    }

    void IceBlock::Update(float deltaTime)
    {
        if (m_isActive)
        {
            if (m_state == State::Crushing)
            {
                m_crushTimer += deltaTime;
                if (m_crushTimer >= CRUSH_FRAME_TIME)
                {
                    m_crushTimer -= CRUSH_FRAME_TIME;
                    m_crushFrame++;
                    if (m_crushFrame >= CRUSH_FRAMES)
                    {
                        SetActive(false);
                        m_state = State::Idle;
                    }
                }
            }
            else if (m_state == State::ShowingScore)
            {
                // Hold the score for a moment, then settle into a normal ice block
                m_scoreTimer -= deltaTime;
                if (m_scoreTimer <= 0.0f)
                {
                    m_state = State::Idle;
                }
            }
            else if (m_state == State::Sliding)
            {
                const glm::vec3 currentPos = GetLocalPosition();
                const glm::vec3 nextPos = currentPos + glm::vec3(m_slideDirection.x, m_slideDirection.y, 0.0f) * SLIDE_SPEED * deltaTime;

                const auto& grid = ServiceLocator::get_collision_grid();
                
                // Use leading edge for collision detection to avoid overlapping sprites
                glm::vec3 leadingPos = nextPos;
                if (m_slideDirection.x > 0) leadingPos.x += 31.0f; // Right edge
                else if (m_slideDirection.x < 0) leadingPos.x += 0.0f; // Left edge
                else if (m_slideDirection.y > 0) leadingPos.y += 31.0f; // Bottom edge
                else if (m_slideDirection.y < 0) leadingPos.y += 0.0f; // Top edge

                const auto [currentRow, currentCol] = grid.WorldToGrid(currentPos);
                const auto [nextRow, nextCol] = grid.WorldToGrid(leadingPos);

                bool blocked = false;

                // Carry a Sno-Bee that is directly in front of us: shove it along smoothly, and
                // only squash it once it has nowhere left to go (a wall or another ice block).
                const glm::vec3 blockCenter = nextPos + glm::vec3(16.0f, 16.0f, 0.0f);
                for (auto* obj : grid.GetNearbyObjects(nextPos))
                {
                    auto* snoBee = dynamic_cast<SnoBeeCharacter*>(obj);
                    if (!snoBee) continue;

                    const glm::vec3 beePos = snoBee->GetWorldPosition();
                    const glm::vec3 beeCenter = beePos + glm::vec3(16.0f, 16.0f, 0.0f);
                    const glm::vec2 toBee{ beeCenter.x - blockCenter.x, beeCenter.y - blockCenter.y };

                    // Only the Sno-Bee right in front of us, roughly in our lane
                    const float along = toBee.x * m_slideDirection.x + toBee.y * m_slideDirection.y;
                    const float perp = std::abs(toBee.x * m_slideDirection.y) + std::abs(toBee.y * m_slideDirection.x);
                    if (along <= 0.0f || along > 40.0f || perp > 18.0f) continue;

                    // Where it would sit if we push it flush in front of us
                    const glm::vec3 pushedPos = nextPos + glm::vec3(m_slideDirection.x, m_slideDirection.y, 0.0f) * 32.0f;
                    const float pushedAlong = pushedPos.x * m_slideDirection.x + pushedPos.y * m_slideDirection.y;
                    const float beeAlong = beePos.x * m_slideDirection.x + beePos.y * m_slideDirection.y;
                    if (pushedAlong <= beeAlong) continue; // we haven't caught up to it yet

                    // Is the tile it would be pushed into blocked?
                    glm::vec3 pushedLead = pushedPos;
                    if (m_slideDirection.x > 0) pushedLead.x += 31.0f;
                    else if (m_slideDirection.y > 0) pushedLead.y += 31.0f;
                    const auto [pr, pc] = grid.WorldToGrid(pushedLead);

                    bool aheadBlocked = !grid.IsWithinBounds(pr, pc);
                    if (!aheadBlocked)
                    {
                        for (auto* o : grid.GetObjectsAt(pr, pc))
                        {
                            if (dynamic_cast<IceBlock*>(o)) { aheadBlocked = true; break; }
                        }
                    }

                    if (aheadBlocked)
                    {
                        // Pinned against the obstacle: squash it, then settle onto its tile and
                        // flash the score there before turning back into a normal block.
                        const auto [beeRow, beeCol] = grid.WorldToGrid(beeCenter);
                        snoBee->CrushFrom(m_slideDirection);
                        SetLocalPosition(grid.GridToWorld(beeRow, beeCol));
                        m_state = State::ShowingScore;
                        m_scoreTimer = SCORE_DISPLAY_TIME;
                    }
                    else
                    {
                        // Carry it along and keep its AI from steering off while shoved
                        snoBee->SetPosition(pushedPos.x, pushedPos.y);
                        snoBee->Stun(0.15f);
                        if (auto* gridComp = snoBee->GetComponent<GridObjectComponent>())
                        {
                            gridComp->SyncToCurrentPosition();
                        }
                    }
                    break; // one passenger at a time
                }

                // If we squashed a Sno-Bee we are done moving this frame
                if (m_state == State::Sliding)
                {
                    // Walls and other ice blocks stop us (Sno-Bees handled above)
                    if (!blocked && (nextRow != currentRow || nextCol != currentCol))
                    {
                        if (!grid.IsWithinBounds(nextRow, nextCol))
                        {
                            blocked = true;
                        }
                        else
                        {
                            for (auto* obj : grid.GetObjectsAt(nextRow, nextCol))
                            {
                                if (obj == this) continue;
                                if (dynamic_cast<SnoBeeCharacter*>(obj)) continue;
                                blocked = true;
                                break;
                            }
                        }
                    }

                    if (blocked)
                    {
                        // Stop, aligned to the tile we are currently in
                        const auto [stopRow, stopCol] = grid.WorldToGrid(currentPos);
                        SetLocalPosition(grid.GridToWorld(stopRow, stopCol));
                        m_state = State::Idle;
                    }
                    else
                    {
                        SetLocalPosition(nextPos);
                    }
                }
            }
            GameObject::Update(deltaTime);
        }
    }

    void IceBlock::Reset()
    {
        SetActive(false);
        SetPosition(-1000.0f, -1000.0f);
        m_state = State::Idle;
        m_crushFrame = 0;
        m_crushTimer = 0.0f;
    }

    void IceBlock::Crush()
    {
        m_state = State::Crushing;
        m_crushFrame = 0;
        m_crushTimer = 0.0f;
    }

    void IceBlock::Slide(const glm::vec2& direction)
    {
        m_state = State::Sliding;
        m_slideDirection = direction;
    }
}
