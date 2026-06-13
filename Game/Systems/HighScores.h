#pragma once
#include <string>
#include <vector>

namespace dae
{
    // One row of the arcade high-score table: a three-letter name and the score it earned.
    struct HighScoreEntry
    {
        std::string name;
        int score{ 0 };
    };

    // The persistent high-score table, kept in a flat text file next to the executable (one
    // "<score> <name>" line per row, best first). Saving only happens here, when a finished run
    // is committed via Insert -- the live HUD HI-SCORE never writes the file.
    class HighScores final
    {
    public:
        static constexpr int kMaxEntries = 10; // table is capped; lowest rows fall off
        static constexpr int kNameLength = 3;  // classic three-initial arcade name

        // Read the table from disk, sorted best-first (missing/empty file -> no rows).
        static std::vector<HighScoreEntry> Load();

        // Best score on the table right now, or 0 when it is empty.
        static int TopScore();

        // Would this score earn a spot? True while the table has room, or once it beats the
        // weakest row. Scores of zero never qualify.
        static bool Qualifies(int score);

        // Commit a finished run: insert (name, score), re-sort, cap to kMaxEntries, write the
        // file, and hand back the new table so the board can be shown immediately.
        static std::vector<HighScoreEntry> Insert(const std::string& name, int score);

    private:
        static void Save(const std::vector<HighScoreEntry>& entries);
    };
}
