#pragma once
#include <cmath>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace dae
{
    class GameObject;

    class CollisionGrid final
    {
    public:
        static bool g_ShowCollisionGrid;
        static constexpr int CELL_SIZE = 32;
        static constexpr int NUM_ROWS = 15;
        static constexpr int NUM_COLS = 13;
        static constexpr int TOTAL_CELLS = NUM_ROWS * NUM_COLS;

        CollisionGrid();
        ~CollisionGrid() = default;

        void RegisterObject(GameObject* pObject);
        void UnregisterObject(GameObject* pObject, const glm::vec3& lastPos);
        void UpdateObject(GameObject* pObject, const glm::vec3& oldPos, const glm::vec3& newPos);

        [[nodiscard]] std::vector<GameObject*> GetObjectsAt(int row, int col) const;
        [[nodiscard]] std::vector<GameObject*> GetObjectsInWorldPos(const glm::vec3& worldPos) const;
        [[nodiscard]] std::vector<GameObject*> GetNearbyObjects(const glm::vec3& worldPos) const;

        void Render() const;

        void SetRenderOffset(float x, float y) { m_renderOffsetX = x; m_renderOffsetY = y; }

        [[nodiscard]] constexpr std::pair<int, int> WorldToGrid(const glm::vec3& worldPos) const
        {
            return {
                static_cast<int>(std::floor((worldPos.y - m_renderOffsetY) / CELL_SIZE)),
                static_cast<int>(std::floor((worldPos.x - m_renderOffsetX) / CELL_SIZE))
            };
        }

        [[nodiscard]] constexpr glm::vec3 GridToWorld(int row, int col) const
        {
            return {
                static_cast<float>(col * CELL_SIZE) + m_renderOffsetX,
                static_cast<float>(row * CELL_SIZE) + m_renderOffsetY,
                0.0f
            };
        }

        [[nodiscard]] static constexpr bool IsWithinBounds(int row, int col)
        {
            return row >= 0 && row < NUM_ROWS && col >= 0 && col < NUM_COLS;
        }

    private:
        std::vector<std::vector<GameObject*>> m_grid;
        float m_renderOffsetX = 0.0f;
        float m_renderOffsetY = 0.0f;

        [[nodiscard]] size_t GetIndex(int row, int col) const
        {
            return static_cast<size_t>(row * NUM_COLS + col);
        }
    };
}
