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

        void Update() override;
        const char *GetDebugName() const override { return "Analog Move"; }
        void DrawInspector() const override;

    private:
        InputManager *m_pInputManager{};
        std::uint32_t m_gamepadIndex{};
        float m_speed{};
        float m_deadzoneScale{1.0f};
    };
}