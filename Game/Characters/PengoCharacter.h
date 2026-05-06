#ifndef PENGO_CHARACTER_H
#define PENGO_CHARACTER_H

#include "Character.h"
#include "PengoState.h"
#include <glm/vec3.hpp>
#include <vector>

namespace dae
{
    class InputManager;
    class ResourceManager;
    class RenderComponent;

    // Enum to represent Pengo's facing direction
    enum class PengoDirection
    {
        Down,
        Left,
        Up,
        Right
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

        // --- Latest Key Priority Movement ---
        void AddMoveInput(PengoDirection direction);
        void RemoveMoveInput(PengoDirection direction);

    private:
        void UpdateRenderComponent();
        void ProcessMovement();

        PengoState* m_pCurrentState;
        PengoState* m_pNextState;
        glm::vec3 m_previousPosition{};

        RenderComponent* m_pRenderComponent;
        PengoDirection m_currentDirection{PengoDirection::Down};
        int m_animationFrame{-1};

        // Stack to track currently held movement keys
        std::vector<PengoDirection> m_activeMoveInputs;
    };
}

#endif // PENGO_CHARACTER_H