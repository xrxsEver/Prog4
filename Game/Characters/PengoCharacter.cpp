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
    constexpr float PENGUIN_MOVE_SPEED = 140.f; // Increased speed to snap faster
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
        const float dt = GameTime::GetInstance().GetDeltaTime();
        glm::vec3 currentPos = GetLocalPosition();

        // 1. Apply velocity if we have a target
        if (m_isMovingToTarget)
        {
            glm::vec3 direction = glm::normalize(m_targetPosition - currentPos);
            currentPos += direction * PENGUIN_MOVE_SPEED * dt;

            // Check if we've reached or overshot the target
            if (glm::distance(currentPos, m_targetPosition) < MOVEMENT_EPSILON || glm::dot(direction, m_targetPosition - currentPos) < 0)
            {
                currentPos = m_targetPosition;
                m_isMovingToTarget = false;
            }
            SetLocalPosition(currentPos);
        }

        // 2. If we are NO LONGER moving (either reached target above, or standing still),
        //    check if the player is holding a key to immediately set a new target in the SAME frame.
        if (!m_isMovingToTarget && !m_activeMoveInputs.empty())
        {
            PengoDirection activeDirection = m_activeMoveInputs.back();
            SetDirection(activeDirection);

            m_targetPosition = currentPos;
            switch (activeDirection)
            {
                case PengoDirection::Up:    m_targetPosition.y -= m_blockSize; break;
                case PengoDirection::Down:  m_targetPosition.y += m_blockSize; break;
                case PengoDirection::Left:  m_targetPosition.x -= m_blockSize; break;
                case PengoDirection::Right: m_targetPosition.x += m_blockSize; break;
            }
            m_isMovingToTarget = true;
        }
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
        // Relying entirely on our boolean instead of checking previous positions.
        // This stops the character from dropping out of the MovingState for 1 frame
        // every time he reaches the edge of a grid cell!
        return m_isMovingToTarget;
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
