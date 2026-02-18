#include "FPSComponent.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "GameTime.h"
#include <format>

dae::FPSComponent::FPSComponent(GameObject *pOwner)
    : Component(pOwner)
{
}

void dae::FPSComponent::Update()
{
    float deltaTime = GameTime::GetInstance().GetDeltaTime();
    m_elapsedTime += deltaTime;
    ++m_frameCount;

    if (m_elapsedTime >= 1.0f)  // update text once per second
    {
        float fps = static_cast<float>(m_frameCount) / m_elapsedTime;
        auto textComp = GetOwner()->GetComponent<TextComponent>();
        if (textComp)
        {
            textComp->SetText(std::format("{:.1f} FPS", fps));
        }
        m_frameCount = 0;
        m_elapsedTime = 0.f;
    }
}
