#pragma once
#include "Component.h"

namespace dae
{
    class TextComponent;

    class FPSComponent final : public Component
    {
    public:
        explicit FPSComponent(GameObject *pOwner);

        void Update() override;
        const char *GetDebugName() const override { return "FPS"; }
        void DrawInspector() const override;

    private:
        TextComponent *m_pTextComponent{};
        int m_frameCount{};
        float m_elapsedTime{};
    };
}
