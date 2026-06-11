#pragma once
#include "Component.h"
#include <memory>
#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <string>
#include "MazeGenerator.h"
#include "IceBlockPool.h"

namespace dae
{
    class ResourceManager;
    class Texture2D;
    class Scene;
    class PengoCharacter;

    struct MazeBlock
    {
        int r, c;
        bool removed;
        TileType type;
        bool isSpawning;
        int spawnAnimationFrame;
        float spawnTimer;
        IceBlock* pPooledBlock = nullptr;
    };

    enum class SpawnStep
    {
        None,
        IceBreaking,
        SnoBeeSpawning,
        Finished
    };

    // High-level flow of a level: intro draw, normal play, then the life-lost sequence
    enum class LevelPhase
    {
        Intro,
        Playing,
        DeathWipe,
        DeathHold
    };

    class MazeDrawingComponent final : public Component
    {
    public:
        // Expose full maze view globally for debugging
        static bool g_ShowFullMaze;

        MazeDrawingComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager, const std::string& levelFile, std::function<void(glm::vec2)> onFinished);
        ~MazeDrawingComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

        // Hook Pengo up so the maze can react when he dies
        void SetPengo(PengoCharacter* pPengo) { m_pPengo = pPengo; }

        const char* GetDebugName() const override { return "Maze Drawing Component"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        glm::vec2 GetScreenPos(int r, int c) const;

        // Phase handlers
        void UpdatePlaying(float deltaTime);
        void UpdateDeathWipe(float deltaTime);
        void UpdateDeathHold(float deltaTime);

        // Life-lost sequence
        void StartDeathSequence();
        void BeginRespawn();
        int ClearSnoBees();          // remove every Sno-Bee, return how many were alive
        void SpawnSnoBees(int count);

        Scene& m_scene;
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_backgroundTexture;
        std::shared_ptr<Texture2D> m_iceBlockTexture;
        std::shared_ptr<Texture2D> m_miscTexture;
        std::shared_ptr<Texture2D> m_pengoTexture;
        
        std::vector<MazeBlock> m_blocks{};
        std::vector<int> m_removalOrder{};
        std::function<void(glm::vec2)> m_onFinished{};
        std::string m_levelFile;
        glm::vec2 m_pengoSpawnPos{0, 0};

        int m_rows = 17;
        int m_cols = 15;
        float m_blockSize = 32.0f;
        float m_offsetX = 0.0f;
        float m_offsetY = 0.0f;

        bool m_isFinished = false;
        float m_timer = 0.0f;
        float m_blockRemoveInterval = 0.05f;
        int m_removedStep = 0;

        SpawnStep m_spawnStep = SpawnStep::None;
        std::vector<int> m_spawnBlockIndices{};
        float m_spawnAnimationTimer = 0.0f;
        int m_spawnAnimationFrame = 0;

        std::unique_ptr<IceBlockPool> m_pIceBlockPool;

        // --- Level flow / life-lost sequence ---
        PengoCharacter* m_pPengo = nullptr;
        LevelPhase m_phase = LevelPhase::Intro;
        float m_wipeProgress = 0.0f;                 // how far the black slide has fallen (pixels)
        float m_holdTimer = 0.0f;
        int m_rememberedSnoBeeCount = 0;
        std::vector<glm::vec3> m_blockSnapshot{};    // remembered ice-block positions

        static constexpr float SPAWN_FRAME_TIME = 0.1f;
        static constexpr float SNOBEE_SPAWN_FRAME_TIME = 0.2f;
        static constexpr float WIPE_SPEED = 600.0f;  // pixels per second for the black slide
        static constexpr float DEATH_HOLD_TIME = 1.5f;
    };
}
