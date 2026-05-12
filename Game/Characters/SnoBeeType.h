#pragma once

namespace dae
{
    enum class AITier
    {
        Basic,
        Intermediate,
        Advanced
    };

    // Immutable Type Object for Sno-Bee breeds
    struct SnoBeeType
    {
        float speed;
        int scoreValue;
        int spriteSheetRowOffset;
        AITier aiTier;
        bool isAggressive;
    };
}
