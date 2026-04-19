#pragma once

#include <cstdint>

#include "Character.h"

namespace dae
{
    enum class PengoState : std::uint8_t
    {
        Idle,
        Walking,
        Dying
    };

    class InputManager;
    class ResourceManager;

    class PengoCharacter final : public Character
    {
    public:
        explicit PengoCharacter(ResourceManager &resourceManager);

        void BindKeyboardControls(InputManager &inputManager);
    };
}