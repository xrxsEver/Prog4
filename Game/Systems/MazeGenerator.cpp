#include "MazeGenerator.h"
#include <cstdint>
#include <fstream>
#include <ostream>
#include <istream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace
{
    // Bump when the on-disk layout below changes; ReadBinary refuses mismatched files.
    constexpr std::uint32_t kBinaryVersion = 1;
    constexpr char kBinaryMagic[4] = { 'P', 'N', 'G', 'L' }; // "PeNGo Level"

    // Native-endian POD dumps — fine here since the cooker runs on the same machine that plays.
    template <typename T>
    void WritePod(std::ostream& os, const T& value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T>
    T ReadPod(std::istream& is)
    {
        T value{};
        is.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }
}

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

    MazeGenerator::GenerationResult MazeGenerator::Load(const std::string& dataPath, const std::string& baseName)
    {
#ifdef NDEBUG
        // Release: prefer the cooked binary; fall back to JSON if it somehow wasn't produced.
        const std::string binPath = dataPath + baseName + ".bin";
        std::ifstream probe{ binPath, std::ios::binary };
        if (probe.good())
        {
            probe.close();
            try { return ReadBinary(binPath); }
            catch (const std::exception&) { /* corrupt/old binary — fall through to JSON */ }
        }
        return LoadFromFile(dataPath + baseName + ".json");
#else
        // Debug: JSON is the source of truth (the cooked binary may lag while you edit levels).
        return LoadFromFile(dataPath + baseName + ".json");
#endif
    }

    void MazeGenerator::WriteBinary(const GenerationResult& result, const std::string& filepath)
    {
        std::ofstream out{ filepath, std::ios::binary | std::ios::trunc };
        if (!out.is_open())
        {
            throw std::runtime_error("Failed to open level binary for writing: " + filepath);
        }

        out.write(kBinaryMagic, sizeof(kBinaryMagic));
        WritePod(out, kBinaryVersion);
        WritePod(out, static_cast<std::int32_t>(result.rows));
        WritePod(out, static_cast<std::int32_t>(result.cols));
        WritePod(out, static_cast<std::int32_t>(result.levelNumber));
        WritePod(out, static_cast<std::int32_t>(result.snoBeeCount));
        WritePod(out, result.snoBeeSpeed);

        // Grid, row-major, one byte per tile.
        for (const auto& row : result.grid)
        {
            for (const TileType tile : row)
            {
                WritePod(out, static_cast<std::uint8_t>(tile));
            }
        }

        // Carving order and every Pengo spawn (the grid alone can't tell 'P' from '#').
        WritePod(out, static_cast<std::int32_t>(result.carvingSequence.size()));
        for (const auto& [r, c] : result.carvingSequence)
        {
            WritePod(out, static_cast<std::int32_t>(r));
            WritePod(out, static_cast<std::int32_t>(c));
        }

        WritePod(out, static_cast<std::int32_t>(result.pengoSpawns.size()));
        for (const auto& [r, c] : result.pengoSpawns)
        {
            WritePod(out, static_cast<std::int32_t>(r));
            WritePod(out, static_cast<std::int32_t>(c));
        }
    }

    MazeGenerator::GenerationResult MazeGenerator::ReadBinary(const std::string& filepath)
    {
        std::ifstream in{ filepath, std::ios::binary };
        if (!in.is_open())
        {
            throw std::runtime_error("Failed to open level binary: " + filepath);
        }

        char magic[sizeof(kBinaryMagic)]{};
        in.read(magic, sizeof(magic));
        for (std::size_t i = 0; i < sizeof(kBinaryMagic); ++i)
        {
            if (magic[i] != kBinaryMagic[i])
            {
                throw std::runtime_error("Not a level binary (bad magic): " + filepath);
            }
        }
        if (ReadPod<std::uint32_t>(in) != kBinaryVersion)
        {
            throw std::runtime_error("Unsupported level binary version: " + filepath);
        }

        GenerationResult result;
        result.rows = ReadPod<std::int32_t>(in);
        result.cols = ReadPod<std::int32_t>(in);
        result.levelNumber = ReadPod<std::int32_t>(in);
        result.snoBeeCount = ReadPod<std::int32_t>(in);
        result.snoBeeSpeed = ReadPod<float>(in);

        result.grid.assign(static_cast<std::size_t>(result.rows),
                           std::vector<TileType>(static_cast<std::size_t>(result.cols), TileType::EMPTY));
        for (int r = 0; r < result.rows; ++r)
        {
            for (int c = 0; c < result.cols; ++c)
            {
                result.grid[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] =
                    static_cast<TileType>(ReadPod<std::uint8_t>(in));
            }
        }

        const std::int32_t carveCount = ReadPod<std::int32_t>(in);
        result.carvingSequence.reserve(static_cast<std::size_t>(carveCount));
        for (std::int32_t i = 0; i < carveCount; ++i)
        {
            const int r = ReadPod<std::int32_t>(in);
            const int c = ReadPod<std::int32_t>(in);
            result.carvingSequence.push_back({ r, c });
        }

        const std::int32_t spawnCount = ReadPod<std::int32_t>(in);
        result.pengoSpawns.reserve(static_cast<std::size_t>(spawnCount));
        for (std::int32_t i = 0; i < spawnCount; ++i)
        {
            const int r = ReadPod<std::int32_t>(in);
            const int c = ReadPod<std::int32_t>(in);
            result.pengoSpawns.push_back({ r, c });
        }
        if (!result.pengoSpawns.empty())
        {
            result.pengoSpawn = result.pengoSpawns.front();
        }

        if (!in)
        {
            throw std::runtime_error("Truncated or unreadable level binary: " + filepath);
        }
        return result;
    }
}
