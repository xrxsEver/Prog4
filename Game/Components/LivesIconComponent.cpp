#include "LivesIconComponent.h"
#include "Character.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "Renderer.h"

namespace dae
{
    LivesIconComponent::LivesIconComponent(GameObject* pOwner, ResourceManager& resourceManager, Character* pCharacter)
        : Component(pOwner), m_pResourceManager(&resourceManager), m_pCharacter(pCharacter)
    {
        m_iconTexture = resourceManager.LoadTexture("LivePengo.png");
    }

    void LivesIconComponent::Render() const
    {
        if (!m_iconTexture || m_pCharacter == nullptr) return;

        const auto& pos = GetOwner()->GetWorldPosition();
        const Rect src{0.0f, 0.0f, ICON_SOURCE_SIZE, ICON_SOURCE_SIZE};

        // Stack one icon per life downwards from the owner position
        for (int i = 0; i < m_pCharacter->health; ++i)
        {
            const float y = pos.y + static_cast<float>(i) * (ICON_DRAW_SIZE + ICON_SPACING);
            Renderer::GetInstance().RenderTexture(*m_iconTexture, src, pos.x, y, ICON_DRAW_SIZE, ICON_DRAW_SIZE);
        }
    }

    std::unique_ptr<Component> LivesIconComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<LivesIconComponent>(pOwner, *m_pResourceManager, m_pCharacter);
    }
}
