#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "BaseEnemy.h"
#include "SnoBeeType.h"

namespace dae
{
    enum class EnemyState : std::uint8_t
    {
        Hatching,
        Wandering,
        Chasing,
        BreakingIce,
        Stunned,
        Dead
    };

    class InputManager;
    class ResourceManager;
    class AnalogStickMoveComponent;

    class SnoBeeCharacter final : public BaseEnemy
    {
    public:
        explicit SnoBeeCharacter(ResourceManager &resourceManager, const SnoBeeType* type);
        ~SnoBeeCharacter() override;

        void BindGamepadControls(InputManager &inputManager, std::uint32_t gamepadIndex);

        // Killed by a sliding ice block; remembers the squash direction for the death sprite
        void CrushFrom(const glm::vec2& squashDirection);
        // Hold still for a moment while a sliding ice block carries us along
        void Stun(float duration);

        std::unique_ptr<SnoBeeCharacter> Clone() const;

        static std::unique_ptr<SnoBeeCharacter> Spawn(const SnoBeeCharacter& prototype, const SnoBeeType* type);

    protected:
        void PerformAction(float dt) override;

    private:
        const SnoBeeType* m_pType{};
        AnalogStickMoveComponent* m_pMoveComponent{};

        EnemyState m_currentState{EnemyState::Hatching};

        // Grid movement variables
        glm::vec2 m_currentDirection{0.0f, 1.0f};
        glm::vec2 m_deathDirection{0.0f, 1.0f}; // direction we were squashed from
        glm::vec3 m_targetPosition{};
        bool m_isMovingToTarget{false};

        // A Sno-Bee can't crush ice when it hatches; it earns the ability after a short random wait
        bool m_canCrushIce{false};
        float m_crushAbilityTimer{0.0f};

        float m_stunTimer{0.0f}; // counts down while a sliding block is carrying us

        // Timers and logic
        int m_thinkTimerFrames{0};
        float m_breakIceTimer{0.0f};
        float m_hatchingTimer{2.0f}; // 2 second spawn animation/delay
        float m_deathTimer{0.0f};    // how long the squashed sprite lingers before removal

        // Walking / dying animation
        float m_animTimer{0.0f};
        int m_animFrame{0};

        static constexpr float DEATH_DISPLAY_TIME = 0.8f;

        static constexpr float m_blockSize = 32.0f;
        static constexpr int m_maxThinkFrames = 15;

        void ChangeState(EnemyState nextState);

        // Sprite animation: pick the row by state, the column by a facing direction
        void UpdateAnimation(float dt);
        int DirectionBaseFrame(const glm::vec2& dir) const;

        void ProcessMovement(float dt);
        void Think();
        void ChasePlayer();
        void Wander();

        std::vector<glm::vec2> GetValidDirections() const;
        glm::vec2 GetRandomValidDirection(const std::vector<glm::vec2>& validDirs, const glm::vec2& preferredDir = {0.f, 0.f}) const;
    };
}
