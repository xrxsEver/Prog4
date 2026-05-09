#include "FPSComponent.h"
#include "GameObject.h"
#include "TextComponent.h"
#include <format>
#include <imgui.h>

dae::FPSComponent::FPSComponent(GameObject *pOwner)
    : Component(pOwner)
{
}

void dae::FPSComponent::Update(float deltaTime)
{
    if (!m_pTextComponent)
    {
        m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
    }

    if (!m_pTextComponent)
    {
        return;
    }

    m_timeSinceLastUpdate += deltaTime;
    if (m_timeSinceLastUpdate >= 0.5f)
    {
        m_timeSinceLastUpdate -= 0.5f;
        const float fps = 1.f / deltaTime;
        m_pTextComponent->SetText(std::format("{:.1f} FPS", fps));
    }
}

std::unique_ptr<dae::Component> dae::FPSComponent::Clone(GameObject* pOwner) const
{
    return std::make_unique<FPSComponent>(pOwner);
}

void dae::FPSComponent::DrawInspector() const
{
    ImGui::Text("Time since last update: %.2f", m_timeSinceLastUpdate);
    ImGui::Text("Text target: %s", m_pTextComponent ? "bound" : "missing");
}
