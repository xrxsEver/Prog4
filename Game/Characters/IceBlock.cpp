#include "IceBlock.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "GridObjectComponent.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "GameTime.h"
#include "SnoBeeCharacter.h"
#include "PengoCharacter.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace dae
{
    // Squashing Sno-Bees with a sliding block scores from the "blue" row of scores.png:
    // 1 caught -> 400, 2 -> 1600, 3 -> 3200, 4+ -> 6400 (row 1, columns 1..4).
    static Rect SquashScoreRect(int count)
    {
        const int idx = std::min(std::max(count, 1), 4);
        return Rect{ static_cast<float>(idx) * 16.0f, 16.0f, 16.0f, 16.0f };
    }

    static int SquashScorePoints(int count)
    {
        switch (std::min(std::max(count, 1), 4))
        {
        case 1:  return 400;
        case 2:  return 1600;
        case 3:  return 3200;
        default: return 6400;
        }
    }
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
                        // and bank those points, just like a Sno-Bee squash does.
                        m_scoreSrcRect = Rect{ 80.0f, 0.0f, 16.0f, 16.0f };
                        m_removeAfterScore = true;
                        m_scoreTimer = SCORE_DISPLAY_TIME;
                        m_state = State::ShowingScore;
                        if (m_pPengo) m_pPengo->AddScore(EGG_BREAK_SCORE);
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

                    // Walk the line of Sno-Bees ahead of us. If it dead-ends at a wall or another
                    // ice block the whole line is pinned and gets squashed together.
                    const auto [beeRow, beeCol] = grid.WorldToGrid(beeCenter);
                    std::vector<SnoBeeCharacter*> line;
                    int lr = beeRow, lc = beeCol;
                    bool pinned = false;
                    while (true)
                    {
                        if (!grid.IsWithinBounds(lr, lc)) { pinned = true; break; } // ran into the wall
                        SnoBeeCharacter* here = nullptr;
                        bool iceAhead = false;
                        for (auto* o : grid.GetObjectsAt(lr, lc))
                        {
                            if (o == this) continue;
                            if (auto* s = dynamic_cast<SnoBeeCharacter*>(o))
                            {
                                if (s->health > 0 && !s->IsMarkedForDelete()) here = s;
                            }
                            else if (dynamic_cast<IceBlock*>(o)) iceAhead = true;
                        }
                        if (iceAhead) { pinned = true; break; }   // line ends against an ice block
                        if (!here)    { pinned = false; break; }  // free tile: the line can still slide
                        line.push_back(here);
                        lr += static_cast<int>(m_slideDirection.y);
                        lc += static_cast<int>(m_slideDirection.x);
                    }

                    if (pinned)
                    {
                        // Squash every Sno-Bee in the line, settle on the nearest one's tile, and
                        // flash the matching blue score there before turning back into a block.
                        for (auto* s : line) s->CrushFrom(m_slideDirection);
                        SetLocalPosition(grid.GridToWorld(beeRow, beeCol));
                        const int n = static_cast<int>(line.size());
                        m_scoreSrcRect = SquashScoreRect(n);
                        m_removeAfterScore = false;
                        m_state = State::ShowingScore;
                        m_scoreTimer = SCORE_DISPLAY_TIME;
                        if (m_pPengo) m_pPengo->AddScore(SquashScorePoints(n));
                    }
                    else
                    {
                        // Free space ahead: carry the nearest one along and stop its AI steering off
                        snoBee->SetPosition(pushedPos.x, pushedPos.y);
                        snoBee->Stun(0.15f);
                        if (auto* gridComp = snoBee->GetComponent<GridObjectComponent>())
                        {
                            gridComp->SyncToCurrentPosition();
                        }
                    }
                    break; // handled the line in front of us
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
        if (m_isDiamond) return;            // diamonds can only be pushed, never crushed
        if (m_state != State::Idle) return; // already crushing/sliding/scoring — ignore repeat presses

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
