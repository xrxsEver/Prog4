#include "SnoBeeCharacter.h"

#include <cmath>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "AnalogStickMoveComponent.h"
#include "AddScoreCommand.h"
#include "Component.h"
#include "GameTime.h"
#include "InputManager.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"
#include "State.h"

namespace
{
    constexpr float g_MoveSpeed{400.0f};
    constexpr float g_MovementEpsilon{0.001f};

    class EnemyIdleState final : public dae::State
    {
    public:
        void OnEnter() override {}
        void OnExit() override {}
        void Update(dae::GameObject &actor, const float dt) override
        {
            (void)actor;
            (void)dt;
        }
    };

    class EnemyWalkingState final : public dae::State
    {
    public:
        void OnEnter() override {}
        void OnExit() override {}
        void Update(dae::GameObject &actor, const float dt) override
        {
            (void)actor;
            (void)dt;
        }
    };

    class EnemyDyingState final : public dae::State
    {
    public:
        void OnEnter() override {}
        void OnExit() override {}
        void Update(dae::GameObject &actor, const float dt) override
        {
            (void)actor;
            (void)dt;
        }
    };

    std::unique_ptr<dae::State> CreateEnemyState(const dae::EnemyState state)
    {
        switch (state)
        {
        case dae::EnemyState::Idle:
            return std::make_unique<EnemyIdleState>();
        case dae::EnemyState::Walking:
            return std::make_unique<EnemyWalkingState>();
        case dae::EnemyState::Dying:
            return std::make_unique<EnemyDyingState>();
        default:
            return std::make_unique<EnemyIdleState>();
        }
    }

    class EnemyStateMachineComponent final : public dae::Component
    {
    public:
        explicit EnemyStateMachineComponent(dae::GameObject *owner)
            : Component(owner), m_previousPosition(owner != nullptr ? owner->GetLocalPosition() : glm::vec3{})
        {
            ChangeState(dae::EnemyState::Idle);
        }

        void Update() override
        {
            auto *owner = GetOwner();
            if (owner == nullptr)
            {
                return;
            }

            const auto *character = dynamic_cast<const dae::Character *>(owner);
            if (character == nullptr)
            {
                return;
            }

            const float deltaTime = dae::GameTime::GetInstance().GetDeltaTime();
            if (m_pCurrentState)
            {
                m_pCurrentState->Update(*owner, deltaTime);
            }

            const auto nextState = DetermineState(*owner, *character);
            if (nextState != m_currentState)
            {
                ChangeState(nextState);
            }

            m_previousPosition = owner->GetLocalPosition();
        }

        const char *GetDebugName() const override { return "Enemy State Machine"; }

    private:
        static bool HasMoved(const dae::GameObject &owner, const glm::vec3 &previousPosition)
        {
            const glm::vec3 delta = owner.GetLocalPosition() - previousPosition;
            return std::fabs(delta.x) > g_MovementEpsilon || std::fabs(delta.y) > g_MovementEpsilon || std::fabs(delta.z) > g_MovementEpsilon;
        }

        dae::EnemyState DetermineState(const dae::GameObject &owner, const dae::Character &character) const
        {
            if (character.GetHealth() <= 0)
            {
                return dae::EnemyState::Dying;
            }

            if (HasMoved(owner, m_previousPosition))
            {
                return dae::EnemyState::Walking;
            }

            return dae::EnemyState::Idle;
        }

        void ChangeState(const dae::EnemyState nextState)
        {
            if (m_pCurrentState)
            {
                m_pCurrentState->OnExit();
            }

            m_pCurrentState = CreateEnemyState(nextState);
            m_currentState = nextState;

            if (m_pCurrentState)
            {
                m_pCurrentState->OnEnter();
            }
        }

        std::unique_ptr<dae::State> m_pCurrentState{};
        dae::EnemyState m_currentState{dae::EnemyState::Idle};
        glm::vec3 m_previousPosition{};
    };
}

dae::SnoBeeCharacter::SnoBeeCharacter(ResourceManager &resourceManager)
    : Character("SnoBee", resourceManager)
{
    InitializeSprite(8 * 16.0f, 9 * 16.0f);
    AddComponent<EnemyStateMachineComponent>();
}

void dae::SnoBeeCharacter::BindGamepadControls(InputManager &inputManager, const std::uint32_t gamepadIndex)
{
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadUp, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, -1.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadDown, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, 1.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadLeft, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{-1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadRight, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::X, KeyState::Down, std::make_unique<LoseLifeCommand>(*this));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::A, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 10));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::B, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 100));
    AddComponent<AnalogStickMoveComponent>(inputManager, gamepadIndex, g_MoveSpeed);
}