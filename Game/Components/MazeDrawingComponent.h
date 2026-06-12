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
#include "SnoBeeType.h"

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

        // Hook Pengo up so the maze can react when he dies, and so blocks can credit kills
        void SetPengo(PengoCharacter* pPengo) { m_pPengo = pPengo; if (m_pIceBlockPool) m_pIceBlockPool->SetPlayer(pPengo); }

        // Co-op: a second Pengo the maze also places, watches for death, and respawns.
        void SetSecondPengo(PengoCharacter* pPengo) { m_pPengo2 = pPengo; }

        // Fired once when the level is cleared (whole reserve spent, no Sno-Bee left alive).
        void SetOnLevelComplete(std::function<void()> cb) { m_onLevelComplete = std::move(cb); }

        // Versus: stop the maze hatching its own Sno-Bees (player two brings the only one).
        void SetEnemiesEnabled(bool enabled) { m_enemiesEnabled = enabled; if (!enabled) m_snoBeeReserve = 0; }

        // Where the second player starts (the level's second 'P' tile), if there is one.
        glm::vec2 GetSecondPengoSpawn() const { return m_pengoSpawnPos2; }
        bool HasSecondSpawn() const { return m_hasSecondSpawn; }

        // How many Sno-Bees are still waiting in the reserve (drives the on-screen counter)
        int GetSnoBeeReserve() const { return m_snoBeeReserve; }

        // Current level number, read from the level JSON's "level" field
        int GetLevelNumber() const { return m_levelNumber; }

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
        bool AnySnoBeeAlive() const; // is any Sno-Bee still on the field (incl. dying ones)?
        void SpawnSnoBees(int count);

        // Reserve Sno-Bees wait as red eggs on the ice, then hatch one at a time with the spawn animation
        bool AdvanceSpawnAnimation(float deltaTime); // steps the ice-break / hatch animation, spawns on finish
        void SetupReserveEggs(int count);            // scatter `count` reserve Sno-Bees as blinking red eggs
        void BeginReserveHatch();                    // hatch one egg with the spawn animation, removing its ice block
        void UpdateRedFlash(float deltaTime);

        // Diamond blocks: push-only blocks that grant a bonus when lined up three in a row
        void SetupDiamonds();                        // mark three ice blocks as diamonds
        void CheckDiamondLine();                     // award the bonus + stun when the diamonds line up
        void StunAllSnoBees(float duration);

        Scene& m_scene;
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_backgroundTexture;
        std::shared_ptr<Texture2D> m_iceBlockTexture;
        std::shared_ptr<Texture2D> m_miscTexture;
        std::shared_ptr<Texture2D> m_pengoTexture;
        
        std::vector<MazeBlock> m_blocks{};
        std::vector<int> m_removalOrder{};
        std::function<void(glm::vec2)> m_onFinished{};
        std::function<void()> m_onLevelComplete{};
        bool m_levelComplete = false;        // fired the one-shot clear callback already?
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
        PengoCharacter* m_pPengo2 = nullptr;             // co-op second player (optional)
        glm::vec2 m_pengoSpawnPos2{0, 0};
        bool m_hasSecondSpawn = false;
        bool m_enemiesEnabled = true;                    // versus turns the AI Sno-Bee spawns off
        LevelPhase m_phase = LevelPhase::Intro;
        float m_wipeProgress = 0.0f;                 // how far the black slide has fallen (pixels)
        float m_holdTimer = 0.0f;
        int m_rememberedSnoBeeCount = 0;
        std::vector<glm::vec3> m_blockSnapshot{};    // remembered ice-block positions

        // Per-level tuning, all read from the level JSON in the constructor
        int m_levelNumber = 1;
        int m_totalSnoBees = 12;          // size of this level's Sno-Bee pool
        SnoBeeType m_levelSnoBeeType{};   // the breed every Sno-Bee spawns as (speed + sprite per level)

        // The first wave hatches up to OPENING_WAVE Sno-Bees, the rest wait in reserve
        static constexpr int OPENING_WAVE = 3;
        int m_snoBeeReserve = m_totalSnoBees;

        // The reserve trickles out one at a time: sit idle, flash all eggs for a moment, then hatch one
        float m_hatchTimer = HATCH_INTERVAL - PRE_SPAWN_FLASH;
        static constexpr float HATCH_INTERVAL = 8.0f;   // seconds between reserve hatches
        static constexpr float PRE_SPAWN_FLASH = 2.0f;  // eggs flash this long before one hatches

        std::vector<int> m_eggBlocks{};       // ice tiles holding un-hatched reserve Sno-Bees
        bool m_hatchingReserve = false;       // is the running spawn animation a reserve hatch (not the intro)?

        // Diamond blocks (push-only); lining all three up grants a bonus and stuns every Sno-Bee
        std::vector<IceBlock*> m_diamondBlocks{};
        std::vector<glm::vec3> m_diamondSnapshot{}; // remembered diamond positions across a respawn
        bool m_diamondLineActive = false;           // currently lined up (so we only award once per line-up)
        static constexpr int DIAMOND_COUNT = 3;
        static constexpr int DIAMOND_BONUS = 5000;
        static constexpr float DIAMOND_STUN_TIME = 6.0f;
        bool m_eggsFlashing = false;          // are the eggs in their pre-spawn flash window?
        float m_flashWindowTimer = 0.0f;
        float m_redFlashTimer = 0.0f;
        bool m_redFlashOn = false;
        static constexpr float RED_FLASH_INTERVAL = 0.25f;

        static constexpr float SPAWN_FRAME_TIME = 0.1f;
        static constexpr float SNOBEE_SPAWN_FRAME_TIME = 0.2f;
        static constexpr float WIPE_SPEED = 600.0f;  // pixels per second for the black slide
        static constexpr float DEATH_HOLD_TIME = 1.5f;
    };
}
