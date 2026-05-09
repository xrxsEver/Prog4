#include "SnoBeeCharacter.h"
#include "GameObject.h"

#include <cmath>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <thread>

#include "AnalogStickMoveComponent.h"
#include "AddScoreCommand.h"
#include "Component.h"
#include "GameTime.h"
#include "InputManager.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"
#include "State.h"
#include "RenderComponent.h"

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

    // Per.7: Design to Enable Optimization.
    // We separate the data needed for AI decision-making into a small, cache-friendly struct.
    struct AIUpdateData
    {
        int health;
        glm::vec3 currentPosition;
        glm::vec3 previousPosition;
    };

    class EnemyStateMachineComponent final : public dae::Component
    {
    public:
        explicit EnemyStateMachineComponent(dae::GameObject *owner)
            : Component(owner), m_previousPosition(owner != nullptr ? owner->GetLocalPosition() : glm::vec3{})
        {
            ChangeState(dae::EnemyState::Idle);
        }

        std::unique_ptr<dae::Component> Clone(dae::GameObject* pOwner) const override
        {
            auto clone = std::make_unique<EnemyStateMachineComponent>(pOwner);
            clone->m_currentState = m_currentState;
            clone->m_previousPosition = m_previousPosition;
            // This is tricky, we might need a deep copy of states if they hold data
            return clone;
        }

        void Update(float deltaTime) override
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

            if (m_pCurrentState)
            {
                m_pCurrentState->Update(*owner, deltaTime);
            }

            AIUpdateData updateData{
                character->health,
                owner->GetLocalPosition(),
                m_previousPosition
            };

            dae::EnemyState nextState = m_currentState;

            std::jthread aiThread([&nextState, updateData]() {
                nextState = DetermineState(updateData);
            });
            // Automatically joins upon destruction of the jthread,
            // ensuring the state is determined before we proceed.
            // Wait for thread to finish so nextState is valid
            aiThread.join();

            if (nextState != m_currentState)
            {
                ChangeState(nextState);
            }

            m_previousPosition = owner->GetLocalPosition();
        }

        const char *GetDebugName() const override { return "Enemy State Machine"; }

    private:
        static dae::EnemyState DetermineState(const AIUpdateData& data)
        {
            if (data.health <= 0)
            {
                return dae::EnemyState::Dying;
            }

            const glm::vec3 delta = data.currentPosition - data.previousPosition;
            const bool hasMoved = std::fabs(delta.x) > g_MovementEpsilon ||
                                  std::fabs(delta.y) > g_MovementEpsilon ||
                                  std::fabs(delta.z) > g_MovementEpsilon;

            if (hasMoved)
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

dae::SnoBeeCharacter::~SnoBeeCharacter() = default;

void dae::SnoBeeCharacter::BindGamepadControls(InputManager &inputManager, const std::uint32_t gamepadIndex)
{
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadUp, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, -1.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadDown, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, 1.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadLeft, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{-1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadRight, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::X, KeyState::Down, std::make_unique<LoseLifeCommand>(*this));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::A, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 10));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::B, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 100));
    m_pMoveComponent = AddComponent<AnalogStickMoveComponent>(inputManager, gamepadIndex, g_MoveSpeed);
}

void dae::SnoBeeCharacter::ApplyConfig(const SnoBeeConfig& config)
{
    m_currentConfig = config;
    score = config.scoreValue;

    if (m_pMoveComponent)
    {
        m_pMoveComponent->SetSpeed(config.speed);
    }

    if (m_pRenderComponent)
    {
        m_pRenderComponent->SetSourceRect(8 * 16.0f, (9 + config.spriteSheetRowOffset) * 16.0f, 16.0f, 16.0f);
    }
}

std::unique_ptr<dae::SnoBeeCharacter> dae::SnoBeeCharacter::Clone() const
{
    auto clone = std::make_unique<SnoBeeCharacter>(m_resourceManager);
    clone->ApplyConfig(m_currentConfig);
    // Note: We don't automatically clone input bindings here, they are assumed to be separate.
    return clone;
}

std::unique_ptr<dae::SnoBeeCharacter> dae::SnoBeeCharacter::Spawn(const SnoBeeCharacter& prototype, const SnoBeeConfig& config)
{
    auto cloned = prototype.Clone();
    cloned->ApplyConfig(config);
    return cloned;
}
