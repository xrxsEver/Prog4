#pragma once

#include <cstdint>

#include "Component.h"

namespace dae
{
    class InputManager;

    class AnalogStickMoveComponent final : public Component
    {
    public:
        AnalogStickMoveComponent(GameObject *pOwner, InputManager &inputManager, std::uint32_t gamepadIndex, float speed, float deadzoneScale = 1.0f);

        void Update(float deltaTime) override;

        const char *GetDebugName() const override { return "Analog Stick Move"; }
        void DrawInspector() const override;
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

        void SetSpeed(float speed) { m_speed = speed; }
        float GetSpeed() const { return m_speed; }

    private:
        InputManager *m_pInputManager{};
        std::uint32_t m_gamepadIndex{};
        float m_speed{};
        float m_deadzoneScale{};
    };
}
