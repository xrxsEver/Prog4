#include "RotationComponent.h"
#include "GameObject.h"
#include "GameTime.h"
#include <cmath>
#include <glm/vec3.hpp>

dae::RotationComponent::RotationComponent(GameObject *pOwner, float radius, float speed)
    : Component(pOwner), m_radius(radius), m_speed(speed)
{
}

void dae::RotationComponent::Update()
{
    m_angle += m_speed * GameTime::GetInstance().GetDeltaTime();

    const glm::vec3 offset{m_radius * std::cos(m_angle),
                           m_radius * std::sin(m_angle),
                           0.f};

    GetOwner()->SetLocalPosition(offset);
}
