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

    private:
        TextComponent *m_pTextComponent{};
        int m_frameCount{};
        float m_elapsedTime{};
    };
}
