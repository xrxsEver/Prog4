#pragma once

namespace dae
{
    enum class AITier
    {
        Basic,
        Intermediate,
        Advanced
    };

    struct SnoBeeConfig
    {
        float speed{};
        int scoreValue{};
        int spriteSheetRowOffset{};
        AITier aiTier{};
    };
}
