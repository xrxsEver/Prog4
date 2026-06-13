#pragma once
#include <functional>
#include <string>
#include <vector>
#include "GameMode.h"

namespace dae
{
    class SceneManager;
    class ResourceManager;
    class InputManager;
    class Scene;
    class StartMenuComponent;
    class NameEntryComponent;
    class Character;

    // Owns the high-level flow: it draws the start menu and, when a mode is picked, tears the
    // game scene down and rebuilds it for that mode. Switching scenes and rebinding input must
    // happen outside the input-dispatch loop, so requests are deferred and carried out in Tick().
    class GameController final
    {
    public:
        GameController(SceneManager& sceneManager, ResourceManager& resourceManager,
                       InputManager& inputManager, Scene& menuScene, Scene& gameScene, Scene& scoreScene);

        void SetMenu(StartMenuComponent* pMenu) { m_pMenu = pMenu; }

        // Bind keyboard + gamepad navigation to the menu (also re-bound when returning to it).
        void BindMenuInput();

        // Safe to call from inside a Command; the real work happens on the next Tick().
        void RequestMode(GameMode mode);
        void RequestNextLevel();
        void RequestMenu();
        // End-of-run: the last life was lost (completed=false) or all levels were cleared (true).
        void RequestEndRun(bool completed);
        // After name entry, show the table with the freshly committed row highlighted.
        void RequestShowBoard(const std::string& name, int score);

        // Pumped every frame by the active scene; performs any pending transition.
        void Tick();

        // Each mode runs through this many levels (level1..3.json per mode).
        static constexpr int kLevelsPerMode = 3;

    private:
        enum class Pending { None, EnterMode, NextLevel, ShowMenu, EndRun, ShowBoard };

        void EnterMode(GameMode mode);
        void NextLevel();
        void LoadLevel(GameMode mode, int level); // tear down + rebuild the game scene
        // Defer a teardown/rebuild to the end of the active scene's update (never mid-iteration).
        void ScheduleRebuild(std::function<void()> action);
        void ShowMenu();
        void BuildGameScene(GameMode mode, int level);
        void BindReturnToMenu();
        // F2 toggles master mute; bound into every scene so it works menu-wide and in-game.
        void BindMuteToggle();

        // End-of-run flow: capture the score, then show name entry (if it makes the table) or the
        // board, all built into m_scoreScene.
        void EndRun(bool completed);
        void BuildNameEntryScene(int score, bool completed);
        void BuildScoreBoardScene(const std::string& highlightName, int score);
        void BindNameEntryInput(NameEntryComponent* entry);
        void BindBoardInput();
        int CaptureScore() const; // sum of the players whose scores the HUD was showing
        int CaptureLives() const; // shared life pool carried between levels of one run

        // The level file for a (mode, level) pair, e.g. ("Versus", 2) -> "versus2.json".
        static const char* LevelFile(GameMode mode, int level);

        SceneManager& m_sceneManager;
        ResourceManager& m_resourceManager;
        InputManager& m_inputManager;
        Scene& m_menuScene;
        Scene& m_gameScene;
        Scene& m_scoreScene;
        StartMenuComponent* m_pMenu{ nullptr };

        Pending m_pending{ Pending::None };
        GameMode m_pendingMode{ GameMode::SinglePlayer };
        bool m_pendingCompleted{ false };          // EndRun: cleared all levels vs. lost last life
        std::string m_boardHighlightName{};        // ShowBoard: row to highlight (empty = none)
        int m_boardScore{ 0 };

        // Which mode/level is on screen now (drives F1 level-skip progression).
        GameMode m_currentMode{ GameMode::SinglePlayer };
        int m_currentLevel{ 1 };

        // A run spans all three levels: score accumulates and lives are a single pool that carries
        // from one level to the next (each level rebuilds the players, so we seed them from these).
        static constexpr int kStartLives = 5; // matches Character's starting health
        int m_runScore{ 0 };
        int m_runLives{ kStartLives };

        // The players whose scores feed the HUD this run; summed for the final score at run end.
        std::vector<Character*> m_scoreChars{};
    };
}
