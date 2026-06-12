#include "VersusControllerComponent.h"

#include <algorithm>
#include <glm/vec3.hpp>

#include "GameObject.h"
#include "Scene.h"
#include "Renderer.h"
#include "InputManager.h"
#include "CallbackCommand.h"
#include "SnoBeeCharacter.h"

namespace dae
{
    VersusControllerComponent::VersusControllerComponent(GameObject* owner, Scene& scene, InputManager& inputManager, std::uint32_t gamepadIndex)
        : Component(owner)
        , m_scene(scene)
        , m_inputManager(inputManager)
        , m_gamepadIndex(gamepadIndex)
    {
        // The gamepad talks to the controller, never to a specific Sno-Bee, so a hand-off needs
        // no rebinding: the controller just forwards held input to whoever it currently drives.
        const auto bindDir = [&inputManager, gamepadIndex, this](Gamepad::Button button, glm::vec2 dir)
        {
            inputManager.BindGamepadCommand(gamepadIndex, button, KeyState::Pressed,
                std::make_unique<CallbackCommand>([this, dir] { AddInputDir(dir); }));
            inputManager.BindGamepadCommand(gamepadIndex, button, KeyState::Up,
                std::make_unique<CallbackCommand>([this, dir] { RemoveInputDir(dir); }));
        };

        bindDir(Gamepad::Button::DPadUp,    glm::vec2{ 0.0f, -1.0f });
        bindDir(Gamepad::Button::DPadDown,  glm::vec2{ 0.0f,  1.0f });
        bindDir(Gamepad::Button::DPadLeft,  glm::vec2{ -1.0f, 0.0f });
        bindDir(Gamepad::Button::DPadRight, glm::vec2{  1.0f, 0.0f });
    }

    void VersusControllerComponent::Update(float /*deltaTime*/)
    {
        // Keep control on a living Sno-Bee. The moment ours dies (or is gone), grab another.
        const bool needNew = (m_pControlled == nullptr)
            || m_pControlled->IsMarkedForDelete()
            || m_pControlled->IsDead();
        if (!needNew) return;

        if (SnoBeeCharacter* next = FindControllableSnoBee())
        {
            TakeControl(next);
        }
        else if (m_pControlled && m_pControlled->IsMarkedForDelete())
        {
            m_pControlled = nullptr; // its body is about to be swept this frame; don't dangle
        }
    }

    void VersusControllerComponent::AddInputDir(const glm::vec2& dir)
    {
        if (std::find(m_heldInputs.begin(), m_heldInputs.end(), dir) == m_heldInputs.end())
        {
            m_heldInputs.push_back(dir);
        }
        if (m_pControlled) m_pControlled->AddInputDir(dir);
    }

    void VersusControllerComponent::RemoveInputDir(const glm::vec2& dir)
    {
        m_heldInputs.erase(std::remove(m_heldInputs.begin(), m_heldInputs.end(), dir), m_heldInputs.end());
        if (m_pControlled) m_pControlled->RemoveInputDir(dir);
    }

    SnoBeeCharacter* VersusControllerComponent::FindControllableSnoBee() const
    {
        SnoBeeCharacter* firstReady = nullptr; // a Sno-Bee already past its hatch (can move now)
        SnoBeeCharacter* firstAny = nullptr;   // fallback: anything alive, even mid-hatch

        for (const auto& obj : m_scene.GetObjects())
        {
            auto* snoBee = dynamic_cast<SnoBeeCharacter*>(obj.get());
            if (snoBee == nullptr || snoBee == m_pControlled) continue;
            if (snoBee->IsMarkedForDelete() || snoBee->IsDead()) continue;
            if (snoBee->IsPlayerControlled()) continue;

            if (firstAny == nullptr) firstAny = snoBee;
            if (!snoBee->IsHatching())
            {
                firstReady = snoBee;
                break;
            }
        }
        return firstReady ? firstReady : firstAny;
    }

    void VersusControllerComponent::TakeControl(SnoBeeCharacter* snoBee)
    {
        if (m_pControlled && m_pControlled != snoBee)
        {
            m_pControlled->SetPlayerControlled(false);
        }

        m_pControlled = snoBee;
        if (m_pControlled == nullptr) return;

        m_pControlled->SetPlayerControlled(true);
        // Carry the currently held direction over so a hand-off doesn't drop player two's input
        for (const glm::vec2& dir : m_heldInputs)
        {
            m_pControlled->AddInputDir(dir);
        }
    }

    void VersusControllerComponent::Render() const
    {
        if (m_pControlled == nullptr || m_pControlled->IsDead()) return;

        // Sno-Bees are 32px; ring the body so it reads as "this one is player two's".
        const glm::vec3 pos = m_pControlled->GetWorldPosition();
        const float cx = pos.x + 16.0f;
        const float cy = pos.y + 16.0f;

        auto& renderer = Renderer::GetInstance();
        const Color ring{ 255, 230, 40, 255 }; // bright yellow marker
        renderer.RenderCircle(cx, cy, 17.0f, ring);
        renderer.RenderCircle(cx, cy, 18.5f, ring);
    }

    std::unique_ptr<Component> VersusControllerComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<VersusControllerComponent>(pOwner, m_scene, m_inputManager, m_gamepadIndex);
    }
}
