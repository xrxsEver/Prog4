#pragma once
#include "Component.h"
#include "Renderer.h" // for dae::Rect
#include <memory>

namespace dae
{
    class ResourceManager;
    class Texture2D;

    // Flashes a score sprite from scores.png over its owner for a short while (e.g. the
    // points a stomped Sno-Bee is worth). Reusable wherever a quick score pop is needed.
    class ScorePopupComponent final : public Component
    {
    public:
        ScorePopupComponent(GameObject* owner, ResourceManager& resourceManager);
        ~ScorePopupComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

        void Show(const Rect& srcRect, float duration);

        const char* GetDebugName() const override { return "Score Popup"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_scoresTexture;

        Rect m_srcRect{ 0.0f, 0.0f, 16.0f, 16.0f };
        float m_timer{ 0.0f };
        bool m_active{ false };

        static constexpr float DRAW_SIZE = 32.0f;
    };
}
