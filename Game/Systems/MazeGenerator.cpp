#include "MazeGenerator.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace dae
{
    MazeGenerator::GenerationResult MazeGenerator::LoadFromFile(const std::string& filepath)
    {
        std::ifstream file{filepath};
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open level file: " + filepath);
        }

        const nlohmann::json jsonData = nlohmann::json::parse(file);
        
        const auto& layout = jsonData["layout"];
        const int rows = static_cast<int>(layout.size());
        const int cols = rows > 0 ? static_cast<int>(layout[0].get<std::string>().size()) : 0;

        GenerationResult result;
        result.grid = std::vector<std::vector<TileType>>(rows, std::vector<TileType>(cols, TileType::EMPTY));
        result.rows = rows;
        result.cols = cols;

        // Per-level tuning lives alongside the layout; missing keys fall back to the defaults
        result.levelNumber = jsonData.value("level", 1);
        result.snoBeeCount = jsonData.value("snoBeeCount", 12);
        result.snoBeeSpeed = jsonData.value("snoBeeSpeed", 200.0f);

        for (int r = 0; r < rows; ++r)
        {
            const std::string rowStr = layout[r].get<std::string>();
            for (int c = 0; c < cols; ++c)
            {
                const char tileChar = rowStr[c];
                if (tileChar == 'W')
                {
                    result.grid[r][c] = TileType::WALL;
                }
                else if (tileChar == 'B')
                {
                    result.grid[r][c] = TileType::ICE;
                }
                else if (tileChar == 'P')
                {
                    result.grid[r][c] = TileType::EMPTY;
                    if (result.pengoSpawns.empty())
                    {
                        result.pengoSpawn = {r, c}; // first spawn drives single-player callers
                    }
                    result.pengoSpawns.push_back({r, c});
                    result.carvingSequence.push_back({r, c});
                }
                else if (tileChar == '#')
                {
                    result.grid[r][c] = TileType::EMPTY;
                    result.carvingSequence.push_back({r, c});
                }
                else
                {
                    // Treat any other character (like space) as EMPTY
                    result.grid[r][c] = TileType::EMPTY;
                    result.carvingSequence.push_back({r, c});
                }
            }
        }

        return result;
    }
}
