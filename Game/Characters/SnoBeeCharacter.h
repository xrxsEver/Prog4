#pragma once

#include <cstdint>

#include "Character.h"

namespace dae
{
    enum class EnemyState : std::uint8_t
    {
        Idle,
        Walking,
        Dying
    };

    class InputManager;
    class ResourceManager;

    class SnoBeeCharacter final : public Character
    {
    public:
        explicit SnoBeeCharacter(ResourceManager &resourceManager);

        void BindGamepadControls(InputManager &inputManager, std::uint32_t gamepadIndex);
    };
}