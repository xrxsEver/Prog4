#include "HighScores.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{
    // Next to the executable (not in Data/, which the build recopies and would clobber on rebuild).
    constexpr const char* HIGHSCORE_FILE = "highscores.txt";
}

namespace dae
{
    std::vector<HighScoreEntry> HighScores::Load()
    {
        std::vector<HighScoreEntry> entries;

        std::ifstream file(HIGHSCORE_FILE);
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            HighScoreEntry e;
            // Names are exactly kNameLength non-space chars, so "<score> <name>" parses cleanly.
            if (iss >> e.score >> e.name)
            {
                entries.push_back(std::move(e));
            }
        }

        std::stable_sort(entries.begin(), entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b) { return a.score > b.score; });
        if (entries.size() > static_cast<std::size_t>(kMaxEntries))
        {
            entries.resize(kMaxEntries);
        }
        return entries;
    }

    int HighScores::TopScore()
    {
        const auto entries = Load();
        return entries.empty() ? 0 : entries.front().score;
    }

    bool HighScores::Qualifies(int score)
    {
        if (score <= 0)
        {
            return false;
        }

        const auto entries = Load();
        if (entries.size() < static_cast<std::size_t>(kMaxEntries))
        {
            return true; // still room on the board
        }
        return score > entries.back().score; // must beat the weakest row
    }

    std::vector<HighScoreEntry> HighScores::Insert(const std::string& name, int score)
    {
        auto entries = Load();
        entries.push_back(HighScoreEntry{ name, score });

        std::stable_sort(entries.begin(), entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b) { return a.score > b.score; });
        if (entries.size() > static_cast<std::size_t>(kMaxEntries))
        {
            entries.resize(kMaxEntries);
        }

        Save(entries);
        return entries;
    }

    void HighScores::Save(const std::vector<HighScoreEntry>& entries)
    {
        std::ofstream file(HIGHSCORE_FILE, std::ios::trunc);
        if (!file)
        {
            return;
        }
        for (const auto& e : entries)
        {
            file << e.score << ' ' << e.name << '\n';
        }
    }
}
