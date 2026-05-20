#include "IceBlock.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "GridObjectComponent.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "GameTime.h"

namespace dae
{
    IceBlock::IceBlock(ResourceManager& resourceManager)
        : GameObject("IceBlock")
    {
        m_pRenderComponent = AddComponent<RenderComponent>(resourceManager);
        m_pRenderComponent->SetTexture("iceblock.png");
        m_pRenderComponent->SetRenderSize(32.0f, 32.0f);
        
        m_miscTexture = resourceManager.LoadTexture("misc.png");

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

                // Check if the leading edge is in a different tile than our current tile
                if (nextRow != currentRow || nextCol != currentCol)
                {
                    if (!grid.IsWithinBounds(nextRow, nextCol))
                    {
                        blocked = true;
                    }
                    else
                    {
                        const auto objects = grid.GetObjectsAt(nextRow, nextCol);
                        for (auto* obj : objects)
                        {
                            if (obj != this)
                            {
                                blocked = true;
                                break;
                            }
                        }
                    }
                }

                if (blocked)
                {
                    // Snap to the tile BEFORE the blocked one
                    int stopRow = nextRow;
                    int stopCol = nextCol;

                    if (m_slideDirection.x > 0) stopCol--;
                    else if (m_slideDirection.x < 0) stopCol++;
                    else if (m_slideDirection.y > 0) stopRow--;
                    else if (m_slideDirection.y < 0) stopRow++;

                    SetLocalPosition(grid.GridToWorld(stopRow, stopCol));
                    m_state = State::Idle;
                }
                else
                {
                    SetLocalPosition(nextPos);
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
