#include "PengoCharacter.h"

#include <cmath>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "InputManager.h"
#include "AddScoreCommand.h"
#include "Component.h"
#include "GameTime.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"
#include "State.h"

namespace
{
    constexpr float g_MoveSpeed{200.0f};
    constexpr float g_MovementEpsilon{0.001f};

    class PengoIdleState final : public dae::State
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

    class PengoWalkingState final : public dae::State
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

    class PengoDyingState final : public dae::State
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

    std::unique_ptr<dae::State> CreatePengoState(const dae::PengoState state)
    {
        switch (state)
        {
        case dae::PengoState::Idle:
            return std::make_unique<PengoIdleState>();
        case dae::PengoState::Walking:
            return std::make_unique<PengoWalkingState>();
        case dae::PengoState::Dying:
            return std::make_unique<PengoDyingState>();
        default:
            return std::make_unique<PengoIdleState>();
        }
    }

    class PengoStateMachineComponent final : public dae::Component
    {
    public:
        explicit PengoStateMachineComponent(dae::GameObject *owner)
            : Component(owner), m_previousPosition(owner != nullptr ? owner->GetLocalPosition() : glm::vec3{})
        {
            ChangeState(dae::PengoState::Idle);
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

            const auto nextState = DetermineState(*owner, *character);
            if (nextState != m_currentState)
            {
                ChangeState(nextState);
            }

            m_previousPosition = owner->GetLocalPosition();
        }

        const char *GetDebugName() const override { return "Pengo State Machine"; }

    private:
        static bool HasMoved(const dae::GameObject &owner, const glm::vec3 &previousPosition)
        {
            const glm::vec3 delta = owner.GetLocalPosition() - previousPosition;
            return std::fabs(delta.x) > g_MovementEpsilon || std::fabs(delta.y) > g_MovementEpsilon || std::fabs(delta.z) > g_MovementEpsilon;
        }

        dae::PengoState DetermineState(const dae::GameObject &owner, const dae::Character &character) const
        {
            if (character.health <= 0)
            {
                return dae::PengoState::Dying;
            }

            if (HasMoved(owner, m_previousPosition))
            {
                return dae::PengoState::Walking;
            }

            return dae::PengoState::Idle;
        }

        void ChangeState(const dae::PengoState nextState)
        {
            if (m_pCurrentState)
            {
                m_pCurrentState->OnExit();
            }

            m_pCurrentState = CreatePengoState(nextState);
            m_currentState = nextState;

            if (m_pCurrentState)
            {
                m_pCurrentState->OnEnter();
            }
        }

        std::unique_ptr<dae::State> m_pCurrentState{};
        dae::PengoState m_currentState{dae::PengoState::Idle};
        glm::vec3 m_previousPosition{};
    };
}

dae::PengoCharacter::PengoCharacter(ResourceManager &resourceManager)
    : Character("Pengo", resourceManager)
{
    InitializeSprite(0.0f, 0.0f);
    AddComponent<PengoStateMachineComponent>();
}

void dae::PengoCharacter::BindKeyboardControls(InputManager &inputManager)
{
    inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, -1.0f}, g_MoveSpeed));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, 1.0f}, g_MoveSpeed));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{-1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{1.0f, 0.0f}, g_MoveSpeed));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_C, KeyState::Down, std::make_unique<LoseLifeCommand>(*this));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_X, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 10));
    inputManager.BindKeyboardCommand(SDL_SCANCODE_V, KeyState::Down, std::make_unique<AddScoreCommand>(*this, 100));
}