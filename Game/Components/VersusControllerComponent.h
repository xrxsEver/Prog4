#pragma once
#include "Component.h"
#include <cstdint>
#include <vector>
#include <glm/vec2.hpp>

namespace dae
{
    class Scene;
    class InputManager;
    class SnoBeeCharacter;

    // Versus: player two never gets a Sno-Bee of their own — they hijack one of the AI swarm.
    // This object owns player two's gamepad, marks the hijacked Sno-Bee with a ring, and the
    // instant Pengo squashes it, hands control to another living Sno-Bee so player two is never
    // benched while any enemy is still on the field.
    class VersusControllerComponent final : public Component
    {
    public:
        VersusControllerComponent(GameObject* owner, Scene& scene, InputManager& inputManager, std::uint32_t gamepadIndex);

        void Update(float deltaTime) override;
        void Render() const override;

        // Bound to the gamepad D-pad; routed to whichever Sno-Bee is currently hijacked.
        void AddInputDir(const glm::vec2& dir);
        void RemoveInputDir(const glm::vec2& dir);

        const char* GetDebugName() const override { return "Versus Controller"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        // Pick a living, uncontrolled Sno-Bee (preferring one that's done hatching).
        SnoBeeCharacter* FindControllableSnoBee() const;
        // Release the old Sno-Bee, take the new one, and re-feed the held direction.
        void TakeControl(SnoBeeCharacter* snoBee);

        Scene& m_scene;
        InputManager& m_inputManager;
        std::uint32_t m_gamepadIndex;
        SnoBeeCharacter* m_pControlled{ nullptr };
        std::vector<glm::vec2> m_heldInputs; // mirror of held D-pad dirs, to seed a freshly-hijacked Sno-Bee
    };
}
