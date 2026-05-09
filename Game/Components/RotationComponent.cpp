#include "RotationComponent.h"
#include <cmath>
#include <imgui.h>
#include "GameObject.h"

dae::RotationComponent::RotationComponent(GameObject *pOwner, float radius, float speed)
    : Component(pOwner), m_radius(radius), m_speed(speed)
{
}

void dae::RotationComponent::Update(float deltaTime)
{
    m_angle += m_speed * deltaTime;

    const float x = std::cos(m_angle) * m_radius;
    const float y = std::sin(m_angle) * m_radius;

    GetOwner()->SetLocalPosition({x, y, 0.0f});
}

std::unique_ptr<dae::Component> dae::RotationComponent::Clone(GameObject* pOwner) const
{
    auto clone = std::make_unique<RotationComponent>(pOwner, m_radius, m_speed);
    clone->m_angle = m_angle;
    return clone;
}

void dae::RotationComponent::DrawInspector() const
{
    ImGui::Text("Angle: %.2f", m_angle);
    ImGui::DragFloat("Radius", const_cast<float *>(&m_radius), 1.0f, 0.0f, 1000.0f);
    ImGui::DragFloat("Speed", const_cast<float *>(&m_speed), 0.1f, -10.0f, 10.0f);
}
