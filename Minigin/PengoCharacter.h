#pragma once

#include "Character.h"

namespace dae
{
    class PengoCharacter final : public Character
    {
    public:
        PengoCharacter();

        void BindKeyboardControls();
    };
}