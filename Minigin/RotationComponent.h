#pragma once
#include "Component.h"

namespace dae
{
    class RotationComponent final : public Component
    {
    public:
        explicit RotationComponent(GameObject *pOwner, float radius = 50.f, float speed = 2.f);
        void Update() override;

    private:
        float m_radius;
        float m_speed;
        float m_angle{};
    };
}
