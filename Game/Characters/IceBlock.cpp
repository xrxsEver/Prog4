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
            else if (m_state == State::EggBreaking)
            {
                if (m_miscTexture)
                {
                    // Egg-break frames live on the 5th row of misc.png
                    const auto& worldPos = GetWorldPosition();
                    Rect animSrc = { m_crushFrame * 16.0f, EGG_BREAK_ROW_Y, 16.0f, 16.0f };
                    Renderer::GetInstance().RenderTexture(*m_miscTexture, animSrc, worldPos.x, worldPos.y, 32.0f, 32.0f);
                }
            }
            else if (m_state == State::ShowingScore)
            {
                if (m_scoresTexture)
                {
                    const auto& worldPos = GetWorldPosition();
                    Renderer::GetInstance().RenderTexture(*m_scoresTexture, m_scoreSrcRect, worldPos.x, worldPos.y, 32.0f, 32.0f);
                }
            }
            else if (m_isDiamond && m_miscTexture)
            {
                // Diamond block (misc.png row 2, first cell) — also drawn while it slides
                const auto& worldPos = GetWorldPosition();
                const Rect diamondSrc = { 0.0f, 16.0f, 16.0f, 16.0f };
                Renderer::GetInstance().RenderTexture(*m_miscTexture, diamondSrc, worldPos.x, worldPos.y, 32.0f, 32.0f);
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
            else if (m_state == State::EggBreaking)
            {
                m_crushTimer += deltaTime;
                if (m_crushTimer >= EGG_BREAK_FRAME_TIME)
                {
                    m_crushTimer -= EGG_BREAK_FRAME_TIME;
                    m_crushFrame++;
                    if (m_crushFrame >= EGG_BREAK_FRAMES)
                    {
                        // Egg finished breaking: flash the 500 (first row, last column of scores.png)
                        m_scoreSrcRect = Rect{ 80.0f, 0.0f, 16.0f, 16.0f };
                        m_removeAfterScore = true;
                        m_scoreTimer = SCORE_DISPLAY_TIME;
                        m_state = State::ShowingScore;
                    }
                }
            }
            else if (m_state == State::ShowingScore)
            {
                // Hold the score for a moment, then settle into a normal ice block (or vanish for an egg)
                m_scoreTimer -= deltaTime;
                if (m_scoreTimer <= 0.0f)
                {
                    if (m_removeAfterScore)
                    {
                        m_removeAfterScore = false;
                        SetActive(false);
                    }
                    m_state = State::Idle;
                }
            }
            else if (m_state == State::Sliding)
            {
                const glm::vec3 currentPos = GetLocalPosition();
                const glm::vec3 nextPos = currentPos + glm::vec3(m_slideDirection.x, m_slideDirection.y, 0.0f) * SLIDE_SPEED * deltaTime;

                const auto& grid = ServiceLocator::get_collision_grid();

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
                        m_scoreSrcRect = Rect{ 16.0f, 16.0f, 16.0f, 16.0f }; // blue 400
                        m_removeAfterScore = false;                          // settle back into a normal block
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
                    // Look at the cell directly ahead of our centre — same test in every direction,
                    // so right/down behave exactly like left/up.
                    const glm::vec3 center = nextPos + glm::vec3(16.0f, 16.0f, 0.0f);
                    const auto [curRow, curCol] = grid.WorldToGrid(center);
                    const int aheadRow = curRow + static_cast<int>(m_slideDirection.y);
                    const int aheadCol = curCol + static_cast<int>(m_slideDirection.x);

                    bool aheadFree = grid.IsWithinBounds(aheadRow, aheadCol);
                    if (aheadFree)
                    {
                        for (auto* obj : grid.GetObjectsAt(aheadRow, aheadCol))
                        {
                            if (obj == this) continue;
                            if (dynamic_cast<SnoBeeCharacter*>(obj)) continue;
                            aheadFree = false;
                            break;
                        }
                    }

                    if (aheadFree)
                    {
                        SetLocalPosition(nextPos);
                    }
                    else
                    {
                        // Next cell is a wall or ice: glide until we line up with our current cell, then stop
                        const glm::vec3 aligned = grid.GridToWorld(curRow, curCol);
                        const float along = nextPos.x * m_slideDirection.x + nextPos.y * m_slideDirection.y;
                        const float alignedAlong = aligned.x * m_slideDirection.x + aligned.y * m_slideDirection.y;
                        if (along >= alignedAlong)
                        {
                            SetLocalPosition(aligned);
                            m_state = State::Idle;
                        }
                        else
                        {
                            SetLocalPosition(nextPos);
                        }
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
        m_hasEgg = false;
        m_isDiamond = false;
    }

    void IceBlock::Crush()
    {
        if (m_isDiamond) return; // diamonds can only be pushed, never crushed

        m_crushFrame = 0;
        m_crushTimer = 0.0f;
        if (m_hasEgg)
        {
            // Pengo smashed a reserve egg: play the egg-break animation, then award 500
            m_hasEgg = false;
            m_state = State::EggBreaking;
        }
        else
        {
            m_state = State::Crushing;
        }
    }

    void IceBlock::Slide(const glm::vec2& direction)
    {
        m_state = State::Sliding;
        m_slideDirection = direction;
    }
}
