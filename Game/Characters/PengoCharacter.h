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
    class BorderComponent;

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

        // Let Pengo rattle the field border when he pushes straight into a wall
        void SetBorder(BorderComponent* pBorder) { m_pBorder = pBorder; }

        // Kick off the dying sequence (ignored if already dying). Pengo then holds the
        // dying animation until the level coordinator calls Respawn().
        void Die();
        bool IsDying() const { return m_isDying; }
        // Lose a life and drop back into play at the given spot, facing down
        void Respawn(const glm::vec3& position);

        bool HasMoved() const;

        void SetDirection(PengoDirection direction);
        PengoDirection GetDirection() const;
        void SetAnimationFrame(int frameIndex);
        void Push();

        void SetSpriteData(int row, int startCol, bool isMoving);

        // --- Latest Key Priority Movement ---
        void AddMoveInput(PengoDirection direction);
        void RemoveMoveInput(PengoDirection direction);

    private:
        void UpdateRenderComponent();
        void ProcessMovement();
        // Look for a living enemy sharing Pengo's tile and die if one is found
        void CheckEnemyCollision();

        std::unique_ptr<PengoState> m_pCurrentState;
        glm::vec3 m_previousPosition{};

        RenderComponent* m_pRenderComponent;
        BorderComponent* m_pBorder{nullptr};
        PengoDirection m_currentDirection{PengoDirection::Down};
        int m_animationFrame{-1};

        // Stack to track currently held movement keys
        std::vector<PengoDirection> m_activeMoveInputs;

        // Grid movement variables
        glm::vec3 m_targetPosition{};
        bool m_isMovingToTarget{false};

        // Death / respawn state
        bool m_isDying{false};

        // float m_animationTimer{0.0f};
        // int m_currentFrame{0};
        int m_spriteRow{0};
        int m_spriteStartCol{0};
        bool m_isMoving{false};

        static constexpr float m_blockSize = 32.0f;
    };
}

#endif // PENGO_CHARACTER_H