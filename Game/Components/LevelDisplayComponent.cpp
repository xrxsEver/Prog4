#include "LevelDisplayComponent.h"
#include <string>

#include "GameObject.h"
#include "TextComponent.h"
#include "MazeDrawingComponent.h"

namespace dae
{
    LevelDisplayComponent::LevelDisplayComponent(GameObject* pOwner, MazeDrawingComponent* pMaze, std::string labelPrefix)
        : Component(pOwner), m_pMaze(pMaze), m_labelPrefix(std::move(labelPrefix))
    {
    }

    void LevelDisplayComponent::Update(float /*deltaTime*/)
    {
        if (m_pTextComponent == nullptr)
        {
            m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
        }
        if (m_pMaze == nullptr || m_pTextComponent == nullptr) return;

        const int level = m_pMaze->GetLevelNumber();
        if (level != m_cached)
        {
            m_cached = level;
            m_pTextComponent->SetText(m_labelPrefix + ": " + std::to_string(level));
        }
    }

    std::unique_ptr<Component> LevelDisplayComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<LevelDisplayComponent>(pOwner, m_pMaze, m_labelPrefix);
    }
}
