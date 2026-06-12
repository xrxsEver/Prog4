#pragma once

namespace dae
{
    // The three playable modes offered by the start menu. The integer values double as the
    // menu row order, so keep them contiguous starting at zero.
    enum class GameMode
    {
        SinglePlayer = 0,
        CoOp = 1,
        Versus = 2
    };

    inline constexpr int kGameModeCount = 3;
}
