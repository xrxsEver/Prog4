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
            std::pair<int, int> pengoSpawn{0, 0};
            int rows{0};
            int cols{0};
        };

        GenerationResult LoadFromFile(const std::string& filepath);

    private:
    };
}
