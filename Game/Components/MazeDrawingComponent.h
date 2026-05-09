#pragma once
#include "Component.h"
#include <memory>
#include <functional>
#include <glm/vec2.hpp>
#include <vector>
#include "MazeGenerator.h"

namespace dae
{
    class ResourceManager;
    class Texture2D;
    class Scene;

    struct MazeBlock
    {
        int r, c;
        bool removed;
        TileType type;
        bool isSpawning;
        int spawnAnimationFrame;
        float spawnTimer;
    };

    enum class SpawnStep
    {
        None,
        IceBreaking,
        SnoBeeSpawning,
        Finished
    };

    class MazeDrawingComponent final : public Component
    {
    public:
        // Expose full maze view globally for debugging
        static bool g_ShowFullMaze;

        MazeDrawingComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager, std::function<void()> onFinished);
        ~MazeDrawingComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

        const char* GetDebugName() const override { return "Maze Drawing Component"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        glm::vec2 GetScreenPos(int r, int c) const;

        Scene& m_scene;
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_backgroundTexture;
        std::shared_ptr<Texture2D> m_iceBlockTexture;
        std::shared_ptr<Texture2D> m_miscTexture;
        std::shared_ptr<Texture2D> m_pengoTexture;
        
        std::vector<MazeBlock> m_blocks{};
        std::vector<int> m_removalOrder{};
        std::function<void()> m_onFinished{};

        int m_rows = 15;
        int m_cols = 13;
        float m_blockSize = 32.0f;
        float m_offsetX = 16.0f;
        float m_offsetY = 16.0f;

        bool m_isFinished = false;
        float m_timer = 0.0f;
        float m_blockRemoveInterval = 0.05f;
        int m_removedStep = 0;

        SpawnStep m_spawnStep = SpawnStep::None;
        std::vector<int> m_spawnBlockIndices{};
        float m_spawnAnimationTimer = 0.0f;
        int m_spawnAnimationFrame = 0;
        
        static constexpr float SPAWN_FRAME_TIME = 0.1f;
        static constexpr float SNOBEE_SPAWN_FRAME_TIME = 0.2f;
    };
}
