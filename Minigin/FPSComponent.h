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
        int m_frameCount{};
        float m_elapsedTime{};
    };
}
