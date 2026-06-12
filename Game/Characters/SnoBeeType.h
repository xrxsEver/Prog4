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
        // Where this breed's sprite block starts in the sheet. The block is laid out the same way
        // for every breed: 8 columns from spriteBaseCol (4 facings x 2 frames), and rows from
        // spriteBaseRow downward — walk, chase (+1), angry (+2), death (+3) — with the hatch/stun
        // frames one row above (spriteBaseRow - 1).
        int spriteBaseRow;
        int spriteBaseCol;
        AITier aiTier;
        bool isAggressive;
    };
}
