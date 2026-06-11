#pragma once
#include <memory>
#include "Component.h"

namespace dae
{
    class Character;
    class ResourceManager;
    class Texture2D;

    // Draws one little Pengo icon per remaining life, stacked in the empty space on the right
    class LivesIconComponent final : public Component
    {
    public:
        LivesIconComponent(GameObject* pOwner, ResourceManager& resourceManager, Character* pCharacter);

        void Render() const override;

        const char* GetDebugName() const override { return "Lives Icon Display"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        ResourceManager* m_pResourceManager{};
        Character* m_pCharacter{};
        std::shared_ptr<Texture2D> m_iconTexture{};

        // One frame of LivePengo.png is 16x16; we crop the leftmost one and draw it bigger
        static constexpr float ICON_SOURCE_SIZE = 16.0f;
        static constexpr float ICON_DRAW_SIZE = 32.0f;
        static constexpr float ICON_SPACING = 6.0f;
    };
}
