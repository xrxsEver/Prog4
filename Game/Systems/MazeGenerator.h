#pragma once
#include <vector>
#include <string>

namespace dae
{
    enum class TileType
    {
        WALL,  // Indestructible outer border
        ICE,   // Destructible blocks
        EMPTY  // Walkable path
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
            std::pair<int, int> pengoSpawn{0, 0};             // first spawn (kept for single-player callers)
            std::vector<std::pair<int, int>> pengoSpawns;     // every 'P' tile, in layout order
            int rows{0};
            int cols{0};

            // Per-level tuning, all read from the JSON (sensible defaults if a key is missing).
            // The Sno-Bee sprite block is chosen in code by level number, not here.
            int levelNumber{1};            // shown on the HUD
            int snoBeeCount{12};           // how many Sno-Bees the level spends in total
            float snoBeeSpeed{200.0f};     // pixels/second the Sno-Bees move
        };

        GenerationResult LoadFromFile(const std::string& filepath);

    private:
    };
}
