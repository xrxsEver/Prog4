#include "FPSComponent.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "GameTime.h"
#include <format>
#include <imgui.h>

dae::FPSComponent::FPSComponent(GameObject *pOwner)
    : Component(pOwner)
{
}

void dae::FPSComponent::Update()
{
    if (!m_pTextComponent)
    {
        m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
    }

    if (!m_pTextComponent)
        return;

    float deltaTime = GameTime::GetInstance().GetDeltaTime();
    m_elapsedTime += deltaTime;
    ++m_frameCount;

    if (m_elapsedTime >= 1.0f) // update text once per second
    {
        float fps = static_cast<float>(m_frameCount) / m_elapsedTime;
        m_pTextComponent->SetText(std::format("{:.1f} FPS", fps));
        m_frameCount = 0;
        m_elapsedTime = 0.f;
    }
}

void dae::FPSComponent::DrawInspector() const
{
    ImGui::Text("Accumulated frames: %d", m_frameCount);
    ImGui::Text("Elapsed: %.3f", m_elapsedTime);
    ImGui::Text("Text target: %s", m_pTextComponent ? "bound" : "missing");
}
