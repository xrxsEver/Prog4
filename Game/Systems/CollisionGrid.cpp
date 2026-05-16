#include "CollisionGrid.h"
#include "GameObject.h"
#include "Renderer.h"
#include <algorithm>

namespace dae
{
    bool CollisionGrid::g_ShowCollisionGrid = false;

    CollisionGrid::CollisionGrid()
    {
        m_grid.resize(TOTAL_CELLS);
    }

    void CollisionGrid::RegisterObject(GameObject* pObject)
    {
        if (!pObject) return;
        const auto [row, col] = WorldToGrid(pObject->GetWorldPosition());
        if (IsWithinBounds(row, col))
        {
            m_grid[GetIndex(row, col)].push_back(pObject);
        }
    }

    void CollisionGrid::UnregisterObject(GameObject* pObject, const glm::vec3& lastPos)
    {
        if (!pObject) return;
        const auto [row, col] = WorldToGrid(lastPos);
        if (IsWithinBounds(row, col))
        {
            auto& cell = m_grid[GetIndex(row, col)];
            cell.erase(std::remove(cell.begin(), cell.end(), pObject), cell.end());
        }
    }

    void CollisionGrid::UpdateObject(GameObject* pObject, const glm::vec3& oldPos, const glm::vec3& newPos)
    {
        if (!pObject) return;
        const auto oldGrid = WorldToGrid(oldPos);
        const auto newGrid = WorldToGrid(newPos);

        if (oldGrid == newGrid) return;

        if (IsWithinBounds(oldGrid.first, oldGrid.second))
        {
            auto& cell = m_grid[GetIndex(oldGrid.first, oldGrid.second)];
            cell.erase(std::remove(cell.begin(), cell.end(), pObject), cell.end());
        }

        if (IsWithinBounds(newGrid.first, newGrid.second))
        {
            m_grid[GetIndex(newGrid.first, newGrid.second)].push_back(pObject);
        }
    }

    std::vector<GameObject*> CollisionGrid::GetObjectsAt(int row, int col) const
    {
        if (IsWithinBounds(row, col))
        {
            return m_grid[GetIndex(row, col)];
        }
        return {};
    }

    std::vector<GameObject*> CollisionGrid::GetObjectsInWorldPos(const glm::vec3& worldPos) const
    {
        const auto [row, col] = WorldToGrid(worldPos);
        return GetObjectsAt(row, col);
    }

    std::vector<GameObject*> CollisionGrid::GetNearbyObjects(const glm::vec3& worldPos) const
    {
        const auto [row, col] = WorldToGrid(worldPos);
        std::vector<GameObject*> nearby;
        
        for (int r = row - 1; r <= row + 1; ++r)
        {
            for (int c = col - 1; c <= col + 1; ++c)
            {
                if (IsWithinBounds(r, c))
                {
                    const auto& cell = m_grid[GetIndex(r, c)];
                    nearby.insert(nearby.end(), cell.begin(), cell.end());
                }
            }
        }
        return nearby;
    }

    void CollisionGrid::Render() const
    {
        if (!g_ShowCollisionGrid) return;

        auto& renderer = Renderer::GetInstance();
        const Color gridColor{ 255, 255, 255, 20 };
        const Color objectColor{ 255, 255, 0, 255 };

        for (int r = 0; r < NUM_ROWS; ++r)
        {
            for (int c = 0; c < NUM_COLS; ++c)
            {
                const glm::vec3 worldPos = GridToWorld(r, c);
                renderer.RenderRect(worldPos.x, worldPos.y, static_cast<float>(CELL_SIZE), static_cast<float>(CELL_SIZE), gridColor);

                const auto& cell = m_grid[GetIndex(r, c)];
                if (!cell.empty())
                {
                    renderer.RenderRect(worldPos.x + 2, worldPos.y + 2, static_cast<float>(CELL_SIZE - 4), static_cast<float>(CELL_SIZE - 4), objectColor);
                }
            }
        }
    }
}
