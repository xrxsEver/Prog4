#pragma once
#include <vector>

namespace dae
{
    enum class TileType
    {
        WALL,  // Indestructible outer border
        ICE,   // Destructible blocks
        EMPTY  // Walkable path
    };

    struct MazeConfig
    {
        int rows{ 15 };
        int cols{ 13 };
        float emptyDensity{ 0.4f }; // 40% EMPTY
    };

    class MazeGenerator final
    {
    public:
        MazeGenerator() = default;
        ~MazeGenerator() = default;

        struct GenerationResult
        {
            std::vector<std::vector<TileType>> grid;
            std::vector<std::pair<int, int>> carvingSequence; // Order in which tiles were made EMPTY
        };

        GenerationResult Generate(const MazeConfig& config);

    private:
        bool IsInside(int r, int c, int rows, int cols) const;
        bool IsSpawnPoint(int r, int c, int rows, int cols) const;
    };
}
