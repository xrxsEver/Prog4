#pragma once

#include <cstdint>

#include "Character.h"

namespace dae
{
    class SnoBeeCharacter final : public Character
    {
    public:
        SnoBeeCharacter();

        void BindGamepadControls(std::uint32_t gamepadIndex);
    };
}