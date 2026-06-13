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

        // Parse a level from its JSON layout (the human-readable, source-of-truth format).
        GenerationResult LoadFromFile(const std::string& filepath);

        // Load a level by base name (e.g. "single1") from a data folder, choosing the format by build:
        // release reads the cooked binary (".bin", falling back to JSON if absent); debug reads JSON,
        // which stays authoritative while you hand-edit levels. dataPath must end in a separator.
        GenerationResult Load(const std::string& dataPath, const std::string& baseName);

        // Binary "cooked" form: a flat dump of a parsed GenerationResult, so loading it is pure
        // deserialization with no text parsing. Produced by the level cooker tool at build time.
        static void WriteBinary(const GenerationResult& result, const std::string& filepath);
        static GenerationResult ReadBinary(const std::string& filepath);

    private:
    };
}
