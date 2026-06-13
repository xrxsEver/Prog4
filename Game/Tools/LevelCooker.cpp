// Build-time asset cooker: turns the human-readable level JSON into the compact binary form the
// release build loads (see MazeGenerator::Load). Run by CMake after the Data folder is staged, so
// the JSON stays the source of truth and the binaries are always regenerated from it.
//
//   level_cooker <data-dir>
//
// Every "*.json" in <data-dir> that parses as a level is written alongside as "*.bin"; anything
// that isn't a level (no "layout") is skipped.

#include "MazeGenerator.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "usage: level_cooker <data-dir>\n";
        return 1;
    }

    const fs::path dataDir = argv[1];
    if (!fs::is_directory(dataDir))
    {
        std::cerr << "level_cooker: data directory not found: " << dataDir.string() << "\n";
        return 1;
    }

    int cooked = 0;
    int skipped = 0;
    for (const auto& entry : fs::directory_iterator(dataDir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
        {
            continue;
        }

        const fs::path jsonPath = entry.path();
        const fs::path binPath = fs::path(jsonPath).replace_extension(".bin");
        try
        {
            dae::MazeGenerator generator;
            const auto result = generator.LoadFromFile(jsonPath.string());
            dae::MazeGenerator::WriteBinary(result, binPath.string());
            ++cooked;
            std::cout << "  cooked " << jsonPath.filename().string()
                      << " -> " << binPath.filename().string() << "\n";
        }
        catch (const std::exception& e)
        {
            // Not every .json is a level; skip the ones that don't parse as one.
            ++skipped;
            std::cout << "  skip   " << jsonPath.filename().string() << " (" << e.what() << ")\n";
        }
    }

    std::cout << "level_cooker: " << cooked << " cooked, " << skipped << " skipped\n";
    return 0;
}
