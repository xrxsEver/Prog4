#include "MazeGenerator.h"
#include <random>
#include <algorithm>
#include <set>

namespace dae
{
    MazeGenerator::GenerationResult MazeGenerator::Generate(const MazeConfig& config)
    {
        GenerationResult result;
        result.grid.assign(config.rows, std::vector<TileType>(config.cols, TileType::ICE));

        // Step 1: Create indestructible border
        for (int r = 0; r < config.rows; ++r)
        {
            for (int c = 0; c < config.cols; ++c)
            {
                if (r == 0 || r == config.rows - 1 || c == 0 || c == config.cols - 1)
                {
                    result.grid[r][c] = TileType::WALL;
                }
            }
        }

        // Target number of EMPTY tiles (inner area: (rows-2)*(cols-2))
        int innerTiles = (config.rows - 2) * (config.cols - 2);
        int targetEmptyCount = static_cast<int>(innerTiles * config.emptyDensity);

        std::vector<std::pair<int, int>> spawnPoints = {
            {config.rows / 2, config.cols / 2}, // Center
            {1, 1},                             // Top-left
            {1, config.cols - 2},                // Top-right
            {config.rows - 2, 1},                // Bottom-left
            {config.rows - 2, config.cols - 2}   // Bottom-right
        };

        std::set<std::pair<int, int>> emptyTiles;
        auto makeEmpty = [&](int r, int c) {
            if (result.grid[r][c] == TileType::ICE)
            {
                result.grid[r][c] = TileType::EMPTY;
                emptyTiles.insert({r, c});
                result.carvingSequence.push_back({r, c});
                return true;
            }
            return false;
        };

        auto canMakeEmpty = [&](int r, int c) {
            if (result.grid[r][c] != TileType::ICE) return false;

            // Prevent 2x2 empty areas
            for (int dr = -1; dr <= 0; ++dr) {
                for (int dc = -1; dc <= 0; ++dc) {
                    int r0 = r + dr;
                    int c0 = c + dc;
                    bool allEmpty = true;
                    for (int i = 0; i <= 1; ++i) {
                        for (int j = 0; j <= 1; ++j) {
                            int rr = r0 + i;
                            int cc = c0 + j;
                            if (rr == r && cc == c) continue;
                            if (!IsInside(rr, cc, config.rows, config.cols) || result.grid[rr][cc] != TileType::EMPTY) {
                                allEmpty = false;
                                break;
                            }
                        }
                        if (!allEmpty) break;
                    }
                    if (allEmpty) return false;
                }
            }
            return true;
        };

        // Step 2 & 5: Clear spawn points and ensure they are connected
        makeEmpty(spawnPoints[0].first, spawnPoints[0].second);
        
        for (size_t i = 1; i < spawnPoints.size(); ++i)
        {
            std::pair<int, int> pos = spawnPoints[i];
            while (result.grid[pos.first][pos.second] != TileType::EMPTY)
            {
                makeEmpty(pos.first, pos.second);
                
                int tr = spawnPoints[0].first;
                int tc = spawnPoints[0].second;
                
                // Move towards center but with some randomness to avoid perfect straight lines
                std::vector<std::pair<int, int>> dirs;
                if (pos.first < tr) dirs.push_back({1, 0});
                if (pos.first > tr) dirs.push_back({-1, 0});
                if (pos.second < tc) dirs.push_back({0, 1});
                if (pos.second > tc) dirs.push_back({0, -1});

                if (dirs.empty()) break;

                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(dirs.begin(), dirs.end(), g);
                
                pos.first += dirs[0].first;
                pos.second += dirs[0].second;
            }
        }

        // Step 3 & 4: Carving with better Path-based Random Walk
        std::random_device rd;
        std::mt19937 gen(rd());
        
        int dr[] = {-1, 1, 0, 0};
        int dc[] = {0, 0, -1, 1};

        while ((int)emptyTiles.size() < targetEmptyCount)
        {
            // Pick a random empty tile to start/restart a path
            std::vector<std::pair<int, int>> emptyList(emptyTiles.begin(), emptyTiles.end());
            std::uniform_int_distribution<> disTile(0, static_cast<int>(emptyList.size()) - 1);
            std::pair<int, int> pos = emptyList[disTile(gen)];

            int walkLength = std::uniform_int_distribution<>(10, 20)(gen);
            int lastDir = -1;

            for (int i = 0; i < walkLength && (int)emptyTiles.size() < targetEmptyCount; ++i)
            {
                std::vector<int> possibleDirs;
                for (int d = 0; d < 4; ++d)
                {
                    int nr = pos.first + dr[d];
                    int nc = pos.second + dc[d];
                    if (IsInside(nr, nc, config.rows, config.cols) && canMakeEmpty(nr, nc))
                    {
                        possibleDirs.push_back(d);
                    }
                }

                if (possibleDirs.empty()) break;

                // Bias towards continuing in the same direction
                int chosenDir = -1;
                if (lastDir != -1 && std::find(possibleDirs.begin(), possibleDirs.end(), lastDir) != possibleDirs.end())
                {
                    // 70% chance to keep same direction
                    if (std::uniform_int_distribution<>(0, 99)(gen) < 70)
                        chosenDir = lastDir;
                }

                if (chosenDir == -1)
                {
                    std::uniform_int_distribution<> disDir(0, static_cast<int>(possibleDirs.size()) - 1);
                    chosenDir = possibleDirs[disDir(gen)];
                }

                pos.first += dr[chosenDir];
                pos.second += dc[chosenDir];
                makeEmpty(pos.first, pos.second);
                lastDir = chosenDir;
            }
        }

        return result;
    }

    bool MazeGenerator::IsInside(int r, int c, int rows, int cols) const
    {
        // Must be within the inner playable area (not the border)
        return r > 0 && r < rows - 1 && c > 0 && c < cols - 1;
    }

    bool MazeGenerator::IsSpawnPoint(int r, int c, int rows, int cols) const
    {
        if (r == rows / 2 && c == cols / 2) return true;
        if (r == 1 && c == 1) return true;
        if (r == 1 && c == cols - 2) return true;
        if (r == rows - 2 && c == 1) return true;
        if (r == rows - 2 && c == cols - 2) return true;
        return false;
    }
}
