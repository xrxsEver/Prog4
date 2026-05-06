#include "PengoCharacter.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "RenderComponent.h"
#include "GameTime.h"
#include "PengoInputCommands.h" // Use the new input commands
#include <cmath>
#include <iostream>
#include <algorithm>

namespace dae
{
    constexpr float PENGUIN_MOVE_SPEED = 100.f;
    constexpr float MOVEMENT_EPSILON = 0.001f;
    constexpr float SPRITE_SIZE = 16.0f;

    // Component to bridge GameObject::Update to PengoCharacter's State Machine
    class PengoUpdateComponent final : public Component
    {
    public:
        explicit PengoUpdateComponent(GameObject* owner) : Component(owner) {}

        void Update(float /*deltaTime*/) override
        {
            if (auto* pengo = dynamic_cast<PengoCharacter*>(GetOwner()))
            {
                pengo->UpdateStateMachine();
            }
        }
    };

    PengoCharacter::PengoCharacter(ResourceManager& resourceManager)
        : Character("Pengo", resourceManager),
          m_pCurrentState(new IdleState()),
          m_pNextState(nullptr),
          m_previousPosition(GetLocalPosition()),
          m_pRenderComponent(nullptr)
    {
        InitializeSprite(0.f, 0.f);

        m_pRenderComponent = GetComponent<RenderComponent>();

        if (m_pRenderComponent)
        {
            SetAnimationFrame(0);
        }

        // Add the bridge component so the state machine updates during GameObject::Update
        AddComponent<PengoUpdateComponent>();

        m_pCurrentState->OnEnter(this);
    }

    PengoCharacter::~PengoCharacter()
    {
        delete m_pCurrentState;
        m_pCurrentState = nullptr;
        delete m_pNextState;
        m_pNextState = nullptr;
    }

    void PengoCharacter::UpdateStateMachine()
    {
        // 1. Process movement based on the active inputs stack
        ProcessMovement();

        // 2. Let states transition
        if (!m_pNextState)
        {
            m_pNextState = m_pCurrentState->Update(this);
        }

        ApplyStateSwap();

        // 3. Update position history for HasMoved() check
        m_previousPosition = GetLocalPosition();
    }

    void PengoCharacter::ProcessMovement()
    {
        // If there are no keys held down, we don't move.
        if (m_activeMoveInputs.empty())
        {
            return;
        }

        // Latest Key Priority: The last key pressed is at the back of the vector.
        PengoDirection activeDirection = m_activeMoveInputs.back();

        // Set the facing direction for animations
        SetDirection(activeDirection);

        // Apply Movement
        const float dt = GameTime::GetInstance().GetDeltaTime();
        glm::vec3 movement{0.0f, 0.0f, 0.0f};

        switch (activeDirection)
        {
            case PengoDirection::Up:    movement.y = -PENGUIN_MOVE_SPEED * dt; break;
            case PengoDirection::Down:  movement.y = PENGUIN_MOVE_SPEED * dt; break;
            case PengoDirection::Left:  movement.x = -PENGUIN_MOVE_SPEED * dt; break;
            case PengoDirection::Right: movement.x = PENGUIN_MOVE_SPEED * dt; break;
        }

        SetLocalPosition(GetLocalPosition() + movement);
    }

    void PengoCharacter::ApplyStateSwap()
    {
        if (m_pNextState != nullptr)
        {
            m_pCurrentState->OnExit(this);
            delete m_pCurrentState;
            m_pCurrentState = m_pNextState;
            m_pNextState = nullptr;
            m_pCurrentState->OnEnter(this);
        }
    }

    void PengoCharacter::BindKeyboardControls(InputManager& inputManager)
    {
        // Key Down - Start tracking input
        inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Down, std::make_unique<MoveInputStartCommand>(*this, PengoDirection::Up));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Down, std::make_unique<MoveInputStartCommand>(*this, PengoDirection::Down));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Down, std::make_unique<MoveInputStartCommand>(*this, PengoDirection::Left));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Down, std::make_unique<MoveInputStartCommand>(*this, PengoDirection::Right));

        // Key Up - Stop tracking input
        inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Up, std::make_unique<MoveInputStopCommand>(*this, PengoDirection::Up));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Up, std::make_unique<MoveInputStopCommand>(*this, PengoDirection::Down));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Up, std::make_unique<MoveInputStopCommand>(*this, PengoDirection::Left));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Up, std::make_unique<MoveInputStopCommand>(*this, PengoDirection::Right));
    }

    void PengoCharacter::AddMoveInput(PengoDirection direction)
    {
        // Avoid adding duplicates if the OS sends repeat keydown events
        if (std::find(m_activeMoveInputs.begin(), m_activeMoveInputs.end(), direction) == m_activeMoveInputs.end())
        {
            m_activeMoveInputs.push_back(direction);
        }
    }

    void PengoCharacter::RemoveMoveInput(PengoDirection direction)
    {
        m_activeMoveInputs.erase(
            std::remove(m_activeMoveInputs.begin(), m_activeMoveInputs.end(), direction),
            m_activeMoveInputs.end()
        );
    }

    bool PengoCharacter::HasMoved() const
    {
        const glm::vec3 delta = GetLocalPosition() - m_previousPosition;
        return std::fabs(delta.x) > MOVEMENT_EPSILON || std::fabs(delta.y) > MOVEMENT_EPSILON;
    }

    void PengoCharacter::SetDirection(PengoDirection direction)
    {
        m_currentDirection = direction;
    }

    PengoDirection PengoCharacter::GetDirection() const
    {
        return m_currentDirection;
    }

    void PengoCharacter::SetAnimationFrame(int frameIndex)
    {
        if (m_animationFrame == frameIndex)
            return;

        m_animationFrame = frameIndex;
        UpdateRenderComponent();
    }

    void PengoCharacter::UpdateRenderComponent()
    {
        if (m_pRenderComponent)
        {
            m_pRenderComponent->SetSourceRect(
                static_cast<float>(m_animationFrame) * SPRITE_SIZE,
                0.0f,
                SPRITE_SIZE,
                SPRITE_SIZE);
        }
    }
}