#include "TimeDisplayComponent.h"
#include <string>

#include "GameObject.h"
#include "TextComponent.h"
#include "MazeDrawingComponent.h"

namespace dae
{
    TimeDisplayComponent::TimeDisplayComponent(GameObject* pOwner, MazeDrawingComponent* pMaze, std::string labelPrefix)
        : Component(pOwner), m_pMaze(pMaze), m_labelPrefix(std::move(labelPrefix))
    {
    }

    void TimeDisplayComponent::Update(float /*deltaTime*/)
    {
        if (m_pTextComponent == nullptr)
        {
            m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
        }
        if (m_pMaze == nullptr || m_pTextComponent == nullptr) return;

        const int seconds = static_cast<int>(m_pMaze->GetLevelTime());
        if (seconds == m_cachedSeconds) return;
        m_cachedSeconds = seconds;

        // M:SS, zero-padded seconds (e.g. "TIME 0:07").
        const int mins = seconds / 60;
        const int secs = seconds % 60;
        const std::string ss = (secs < 10 ? "0" : "") + std::to_string(secs);
        m_pTextComponent->SetText(m_labelPrefix + " " + std::to_string(mins) + ":" + ss);
    }

    std::unique_ptr<Component> TimeDisplayComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<TimeDisplayComponent>(pOwner, m_pMaze, m_labelPrefix);
    }
}
