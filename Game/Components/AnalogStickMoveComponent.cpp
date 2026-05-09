#include "AnalogStickMoveComponent.h"
#include <cmath>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "GameObject.h"
#include "InputManager.h"
#include <imgui.h>

namespace
{
    constexpr float g_Deadzone{0.2f};
}

dae::AnalogStickMoveComponent::AnalogStickMoveComponent(GameObject *pOwner, InputManager &inputManager, const std::uint32_t gamepadIndex, const float speed, const float deadzoneScale)
    : Component(pOwner), m_pInputManager(&inputManager), m_gamepadIndex(gamepadIndex), m_speed(speed), m_deadzoneScale(deadzoneScale)
{
}

void dae::AnalogStickMoveComponent::Update(float deltaTime)
{
    if (m_pInputManager == nullptr)
        return;

    glm::vec2 leftStick{0, 0};
    if (auto state = m_pInputManager->GetGamepadState(m_gamepadIndex))
    {
        // Normalize int16_t to [-1.0, 1.0]
        leftStick.x = state->leftThumbX / 32767.0f;
        leftStick.y = state->leftThumbY / 32767.0f;
    }

    const float length = std::sqrt(leftStick.x * leftStick.x + leftStick.y * leftStick.y);
    const float scaledDeadzone = g_Deadzone * m_deadzoneScale;

    if (length > scaledDeadzone)
    {
        auto *pOwner = GetOwner();
        auto pos = pOwner->GetLocalPosition();
        pos.x += leftStick.x * m_speed * deltaTime;
        // Invert Y axis for movement since SDL Y goes down
        pos.y -= leftStick.y * m_speed * deltaTime;
        pOwner->SetLocalPosition(pos);
    }
}

std::unique_ptr<dae::Component> dae::AnalogStickMoveComponent::Clone(GameObject* pOwner) const
{
    return std::make_unique<AnalogStickMoveComponent>(pOwner, *m_pInputManager, m_gamepadIndex, m_speed, m_deadzoneScale);
}

void dae::AnalogStickMoveComponent::DrawInspector() const
{
    ImGui::Text("Gamepad Index: %u", m_gamepadIndex);
    ImGui::Text("Speed: %.1f", m_speed);
    ImGui::Text("Deadzone Scale: %.2f", m_deadzoneScale);

    if (m_pInputManager)
    {
        glm::vec2 leftStick{0, 0};
        if (auto state = m_pInputManager->GetGamepadState(m_gamepadIndex))
        {
            leftStick.x = state->leftThumbX / 32767.0f;
            leftStick.y = state->leftThumbY / 32767.0f;
        }
        ImGui::Text("Left Stick: %.2f, %.2f", leftStick.x, leftStick.y);
    }
}
