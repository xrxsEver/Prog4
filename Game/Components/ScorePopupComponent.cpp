#include "ScorePopupComponent.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Texture2D.h"

namespace dae
{
    ScorePopupComponent::ScorePopupComponent(GameObject* owner, ResourceManager& resourceManager)
        : Component(owner)
        , m_resourceManager(resourceManager)
    {
        m_scoresTexture = resourceManager.LoadTexture("scores.png");
    }

    void ScorePopupComponent::Show(const Rect& srcRect, float duration)
    {
        m_srcRect = srcRect;
        m_timer = duration;
        m_active = true;
    }

    void ScorePopupComponent::Update(float deltaTime)
    {
        if (!m_active) return;

        m_timer -= deltaTime;
        if (m_timer <= 0.0f) m_active = false;
    }

    void ScorePopupComponent::Render() const
    {
        if (!m_active || !m_scoresTexture) return;

        const auto& pos = GetOwner()->GetWorldPosition();
        Renderer::GetInstance().RenderTexture(*m_scoresTexture, m_srcRect, pos.x, pos.y, DRAW_SIZE, DRAW_SIZE);
    }

    std::unique_ptr<Component> ScorePopupComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<ScorePopupComponent>(pOwner, m_resourceManager);
    }
}
