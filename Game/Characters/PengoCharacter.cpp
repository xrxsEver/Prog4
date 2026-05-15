#include "PengoCharacter.h"
#include "PengoState.h"
#include "RenderComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "MoveReleaseCommand.h"
#include "GameTime.h"
#include <iostream>

namespace dae
{
    constexpr float MOVE_SPEED = 140.0f;
    // constexpr float ANIMATION_SPEED = 0.2f;
    constexpr float SPRITE_SIZE = 16.0f;

    // Component to bridge GameObject::Update to PengoCharacter's State Machine
    class PengoUpdateComponent final : public Component
    {
    public:
        explicit PengoUpdateComponent(GameObject* owner) : Component(owner) {}

        std::unique_ptr<Component> Clone(GameObject* pOwner) const override
        {
            return std::make_unique<PengoUpdateComponent>(pOwner);
        }

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
          m_pCurrentState(std::make_unique<IdleState>()),
          m_previousPosition(GetLocalPosition()),
          m_pRenderComponent(nullptr)
    {
        SetLocalPosition({-1000.f, -1000.f, 0.f});

        InitializeSprite(0.f, 0.f);

        m_pRenderComponent = GetComponent<RenderComponent>();

        if (m_pRenderComponent)
        {
            m_pRenderComponent->SetRenderSize(32.f, 32.f);
        }

        // Add the bridge component so the state machine updates during GameObject::Update
        AddComponent<PengoUpdateComponent>();
    }

    PengoCharacter::~PengoCharacter()
    {
    }

    void PengoCharacter::BindKeyboardControls(InputManager& inputManager)
    {
        // Directional Movement - Pressed
        inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0, -1}, MOVE_SPEED));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0, 1}, MOVE_SPEED));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{-1, 0}, MOVE_SPEED));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{1, 0}, MOVE_SPEED));

        // Directional Movement - Released
        inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Up, std::make_unique<MoveReleaseCommand>(*this, glm::vec2{0, -1}));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Up, std::make_unique<MoveReleaseCommand>(*this, glm::vec2{0, 1}));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Up, std::make_unique<MoveReleaseCommand>(*this, glm::vec2{-1, 0}));
        inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Up, std::make_unique<MoveReleaseCommand>(*this, glm::vec2{1, 0}));


    }

    void PengoCharacter::UpdateStateMachine()
    {
        // Handle movement processing (grid snapping, target reaching)
        ProcessMovement();

        // Determine if we need to change state based on input or logic
        std::unique_ptr<PengoState> nextState = m_pCurrentState->HandleInput(this);

        if (nextState == nullptr)
        {
            nextState = m_pCurrentState->Update(this);
        }

        if (nextState != nullptr)
        {
            m_pCurrentState->OnExit(this);
            m_pCurrentState = std::move(nextState);
            m_pCurrentState->OnEnter(this);
        }

        UpdateRenderComponent();
        // Keep track of previous position for next frame's input inference
        m_previousPosition = GetLocalPosition();
    }

    void PengoCharacter::SetSpriteData(int row, int startCol, bool isMoving)
    {
        m_spriteRow = row;
        m_spriteStartCol = startCol;
        m_isMoving = isMoving;
    }

    void PengoCharacter::UpdateRenderComponent()
    {
        if (m_pRenderComponent)
        {
            if (m_animationFrame != -1)
            {
                m_pRenderComponent->SetSourceRect(
                    m_animationFrame * SPRITE_SIZE,
                    0,
                    SPRITE_SIZE,
                    SPRITE_SIZE
                );
            }
        }
    }

    void PengoCharacter::ApplyStateSwap() {}

    bool PengoCharacter::HasMoved() const { return m_isMovingToTarget || !m_activeMoveInputs.empty(); }

    void PengoCharacter::SetDirection(PengoDirection direction) { m_currentDirection = direction; }
    PengoDirection PengoCharacter::GetDirection() const { return m_currentDirection; }
    void PengoCharacter::SetAnimationFrame(int frameIndex) { m_animationFrame = frameIndex; }

    void PengoCharacter::AddMoveInput(PengoDirection direction)
    {
        auto it = std::find(m_activeMoveInputs.begin(), m_activeMoveInputs.end(), direction);
        if (it == m_activeMoveInputs.end())
        {
            m_activeMoveInputs.push_back(direction);
        }
    }

    void PengoCharacter::RemoveMoveInput(PengoDirection direction)
    {
        auto it = std::find(m_activeMoveInputs.begin(), m_activeMoveInputs.end(), direction);
        if (it != m_activeMoveInputs.end())
        {
            m_activeMoveInputs.erase(it);
        }
    }

    void PengoCharacter::ProcessMovement()
    {
        if (m_isMovingToTarget)
        {
            glm::vec3 currentPos = GetLocalPosition();
            glm::vec3 toTarget = m_targetPosition - currentPos;
            float distance = glm::length(toTarget);
            float moveDist = MOVE_SPEED * GameTime::GetInstance().GetDeltaTime();

            if (moveDist >= distance)
            {
                SetLocalPosition(m_targetPosition);
                m_isMovingToTarget = false;
            }
            else
            {
                SetLocalPosition(currentPos + glm::normalize(toTarget) * moveDist);
            }
        }

        if (!m_isMovingToTarget && !m_activeMoveInputs.empty())
        {
            PengoDirection nextDir = m_activeMoveInputs.back();
            m_currentDirection = nextDir;

            glm::vec3 currentPos = GetLocalPosition();
            glm::vec3 directionVec{0, 0, 0};

            switch (nextDir)
            {
            case PengoDirection::Up:    directionVec.y = -1; break;
            case PengoDirection::Down:  directionVec.y = 1;  break;
            case PengoDirection::Left:  directionVec.x = -1; break;
            case PengoDirection::Right: directionVec.x = 1;  break;
            }

            m_targetPosition = currentPos + directionVec * m_blockSize;
            m_isMovingToTarget = true;
        }
    }
}
