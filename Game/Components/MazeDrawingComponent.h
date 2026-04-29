#pragma once
#include "Component.h"
#include "MazeGenerator.h"
#include <vector>
#include <glm/vec2.hpp>
#include <string>
#include <functional>
#include <memory>

namespace dae
{
    class Texture2D;
    class ResourceManager;

    class MazeDrawingComponent final : public Component
    {
    public:
        MazeDrawingComponent(GameObject* owner, ResourceManager& resourceManager, std::function<void()> onFinished);
        virtual void Update(float deltaTime) override;
        virtual void Render() const override;

        const char* GetDebugName() const override { return "Maze Drawing Component"; }

        static bool g_ShowFullMaze;

    private:
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_texture;
        
        struct Block {
            int r, c;
            bool removed;
            TileType type;
        };
        std::vector<Block> m_blocks;
        std::vector<int> m_removalOrder;
        float m_timer{0.0f};
        float m_blockRemoveInterval{0.05f};
        int m_removedStep{0};
        bool m_isFinished{false};
        std::function<void()> m_onFinished;

        static constexpr int m_rows = 17;
        static constexpr int m_cols = 15;
        
        // Scaled values (factor 2.0 to fit 576px height)
        static constexpr float m_blockSize = 32.0f;
        static constexpr float m_offsetX = 0.0f;
        static constexpr float m_offsetY = 0.0f;

        glm::vec2 GetScreenPos(int r, int c) const;
    };
}
