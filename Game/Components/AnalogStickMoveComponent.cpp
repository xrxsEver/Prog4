#include "AnalogStickMoveComponent.h"

#include <algorithm>
#include <cmath>

#include <glm/vec2.hpp>
#include <imgui.h>

#include "GameObject.h"
#include "GameTime.h"
#include "InputManager.h"

dae::AnalogStickMoveComponent::AnalogStickMoveComponent(GameObject *pOwner, InputManager &inputManager, const std::uint32_t gamepadIndex, const float speed, const float deadzoneScale)
    : Component(pOwner), m_pInputManager(&inputManager), m_gamepadIndex(gamepadIndex), m_speed(speed), m_deadzoneScale(deadzoneScale)
{
}

void dae::AnalogStickMoveComponent::Update(float deltaTime)
{
    if (m_pInputManager == nullptr)
    {
        return;
    }

    const auto gamepadState = m_pInputManager->GetGamepadState(m_gamepadIndex);
    if (!gamepadState)
    {
        return;
    }

    const glm::vec2 rawStick{
        static_cast<float>(gamepadState->leftThumbX),
        static_cast<float>(gamepadState->leftThumbY)};
    const float magnitude = std::sqrt(rawStick.x * rawStick.x + rawStick.y * rawStick.y);
    const float deadzone = static_cast<float>(gamepadState->leftDeadzone) * m_deadzoneScale;
    if (magnitude <= deadzone || magnitude <= 0.0f)
    {
        return;
    }

    const glm::vec2 direction = rawStick / magnitude;
    const float normalizedMagnitude = std::clamp((magnitude - deadzone) / (32767.0f - deadzone), 0.0f, 1.0f);
    const glm::vec3 currentPos = GetOwner()->GetLocalPosition();
    const glm::vec3 movement{
        direction.x * normalizedMagnitude * m_speed * deltaTime,
        -direction.y * normalizedMagnitude * m_speed * deltaTime,
        0.0f};

    GetOwner()->SetLocalPosition(currentPos + movement);
}

void dae::AnalogStickMoveComponent::DrawInspector() const
{
    ImGui::Text("Gamepad index: %u", m_gamepadIndex);
    ImGui::Text("Speed: %.1f", m_speed);
    ImGui::Text("Deadzone scale: %.2f", m_deadzoneScale);
}