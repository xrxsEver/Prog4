#pragma once
#include "Component.h"
#include <string>

namespace dae
{
    class TextComponent;

    class FPSComponent final : public Component
    {
    public:
        explicit FPSComponent(GameObject *pOwner);

        void Update(float deltaTime) override;

        const char *GetDebugName() const override { return "FPS Counter"; }
        void DrawInspector() const override;
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        TextComponent *m_pTextComponent{};
        float m_timeSinceLastUpdate{};
    };
}
