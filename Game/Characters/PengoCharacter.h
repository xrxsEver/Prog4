#ifndef PENGO_CHARACTER_H
#define PENGO_CHARACTER_H

#include "Character.h"
#include <glm/vec3.hpp>
#include <vector>
#include <memory>

namespace dae
{
    class InputManager;
    class ResourceManager;
    class RenderComponent;
    class PengoState;

    // Enum to represent Pengo's facing direction
    enum class PengoDirection
    {
        Down,
        Left,
        Up,
        Right
    };

    enum class PengoInput
    {
        None,
        MoveDown,
        MoveLeft,
        MoveUp,
        MoveRight,
        Push
    };

    class PengoCharacter final : public Character
    {
    public:
        explicit PengoCharacter(ResourceManager& resourceManager);
        ~PengoCharacter() override;

        PengoCharacter(const PengoCharacter& other) = delete;
        PengoCharacter(PengoCharacter&& other) = delete;
        PengoCharacter& operator=(const PengoCharacter& other) = delete;
        PengoCharacter& operator=(PengoCharacter&& other) = delete;

        void UpdateStateMachine();
        void ApplyStateSwap();

        void BindKeyboardControls(InputManager& inputManager);

        bool HasMoved() const;

        void SetDirection(PengoDirection direction);
        PengoDirection GetDirection() const;
        void SetAnimationFrame(int frameIndex);

        void SetSpriteData(int row, int startCol, bool isMoving);

        // --- Latest Key Priority Movement ---
        void AddMoveInput(PengoDirection direction);
        void RemoveMoveInput(PengoDirection direction);

    private:
        void UpdateRenderComponent();
        void ProcessMovement();

        std::unique_ptr<PengoState> m_pCurrentState;
        glm::vec3 m_previousPosition{};

        RenderComponent* m_pRenderComponent;
        PengoDirection m_currentDirection{PengoDirection::Down};
        int m_animationFrame{-1};

        // Stack to track currently held movement keys
        std::vector<PengoDirection> m_activeMoveInputs;

        // Grid movement variables
        glm::vec3 m_targetPosition{};
        bool m_isMovingToTarget{false};

        // float m_animationTimer{0.0f};
        // int m_currentFrame{0};
        int m_spriteRow{0};
        int m_spriteStartCol{0};
        bool m_isMoving{false};

        static constexpr float m_blockSize = 32.0f;
    };
}

#endif // PENGO_CHARACTER_H