#include "PengoCharacter.h"
#include "PengoState.h"
#include "RenderComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "MoveReleaseCommand.h"
#include "PushCommand.h"
#include "GameTime.h"
#include "GridObjectComponent.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "IceBlock.h"
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
            auto* pengo = dynamic_cast<PengoCharacter*>(GetOwner());
            if (!pengo) return;

            pengo->UpdateStateMachine();
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
        AddComponent<GridObjectComponent>();
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

        // Action - Push
        inputManager.BindKeyboardCommand(SDL_SCANCODE_SPACE, KeyState::Down, std::make_unique<PushCommand>(*this));
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
        if (!m_pRenderComponent) return;
        if (m_animationFrame == -1) return;

        m_pRenderComponent->SetSourceRect(
            m_animationFrame * SPRITE_SIZE,
            m_spriteRow * SPRITE_SIZE,
            SPRITE_SIZE,
            SPRITE_SIZE
        );
    }

    void PengoCharacter::Push()
    {
        glm::vec3 directionVec{0, 0, 0};
        switch (m_currentDirection)
        {
        case PengoDirection::Up:    directionVec.y = -1; break;
        case PengoDirection::Down:  directionVec.y = 1;  break;
        case PengoDirection::Left:  directionVec.x = -1; break;
        case PengoDirection::Right: directionVec.x = 1;  break;
        }

        const auto& grid = ServiceLocator::get_collision_grid();
        const glm::vec3 currentPos = GetLocalPosition();
        const glm::vec3 tileAheadPos = currentPos + directionVec * m_blockSize;

        const auto [rowAhead, colAhead] = grid.WorldToGrid(tileAheadPos);
        if (!grid.IsWithinBounds(rowAhead, colAhead)) return;

        IceBlock* pIceBlockAhead = nullptr;
        const auto objectsAhead = grid.GetObjectsAt(rowAhead, colAhead);
        for (auto* obj : objectsAhead)
        {
            if (auto* pBlock = dynamic_cast<IceBlock*>(obj))
            {
                pIceBlockAhead = pBlock;
                break;
            }
        }

        if (pIceBlockAhead)
        {
            const glm::vec3 tileBehindPos = tileAheadPos + directionVec * m_blockSize;
            const auto [rowBehind, colBehind] = grid.WorldToGrid(tileBehindPos);

            bool isOccupied = false;
            if (grid.IsWithinBounds(rowBehind, colBehind))
            {
                const auto objectsBehind = grid.GetObjectsAt(rowBehind, colBehind);
                if (!objectsBehind.empty())
                {
                    isOccupied = true;
                }
            }
            else
            {
                // Out of bounds is treated as occupied (wall)
                isOccupied = true;
            }

            if (isOccupied)
            {
                pIceBlockAhead->Crush();
            }
            else
            {
                pIceBlockAhead->Slide(glm::vec2{directionVec.x, directionVec.y});
            }

            // Transition Pengo to pushing state
            m_pCurrentState->OnExit(this);
            m_pCurrentState = std::make_unique<PushingState>();
            m_pCurrentState->OnEnter(this);
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
            return; // Early return since we are already moving to a target
        }

        if (m_activeMoveInputs.empty()) return;

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
        
        // Check if target position is walkable using CollisionGrid
        const auto& grid = ServiceLocator::get_collision_grid();
        const auto [row, col] = grid.WorldToGrid(m_targetPosition);
        bool canMove = true;
        if (grid.IsWithinBounds(row, col))
        {
            const auto objects = grid.GetObjectsAt(row, col);
            for (auto* obj : objects)
            {
                if (dynamic_cast<IceBlock*>(obj))
                {
                    canMove = false;
                    break;
                }
            }
        }
        else
        {
            canMove = false;
        }

        if (canMove)
        {
            m_isMovingToTarget = true;
        }
        else
        {
            // Blocked
        }
    }
}
