#pragma once
#include <functional>
#include "GameMode.h"

namespace dae
{
    class SceneManager;
    class ResourceManager;
    class InputManager;
    class Scene;
    class StartMenuComponent;

    // Owns the high-level flow: it draws the start menu and, when a mode is picked, tears the
    // game scene down and rebuilds it for that mode. Switching scenes and rebinding input must
    // happen outside the input-dispatch loop, so requests are deferred and carried out in Tick().
    class GameController final
    {
    public:
        GameController(SceneManager& sceneManager, ResourceManager& resourceManager,
                       InputManager& inputManager, Scene& menuScene, Scene& gameScene);

        void SetMenu(StartMenuComponent* pMenu) { m_pMenu = pMenu; }

        // Bind keyboard + gamepad navigation to the menu (also re-bound when returning to it).
        void BindMenuInput();

        // Safe to call from inside a Command; the real work happens on the next Tick().
        void RequestMode(GameMode mode);
        void RequestNextLevel();
        void RequestMenu();

        // Pumped every frame by the active scene; performs any pending transition.
        void Tick();

        // Each mode runs through this many levels (level1..3.json per mode).
        static constexpr int kLevelsPerMode = 3;

    private:
        enum class Pending { None, EnterMode, NextLevel, ShowMenu };

        void EnterMode(GameMode mode);
        void NextLevel();
        void LoadLevel(GameMode mode, int level); // tear down + rebuild the game scene
        // Defer a teardown/rebuild to the end of the active scene's update (never mid-iteration).
        void ScheduleRebuild(std::function<void()> action);
        void ShowMenu();
        void BuildGameScene(GameMode mode, int level);
        void BindReturnToMenu();

        // The level file for a (mode, level) pair, e.g. ("Versus", 2) -> "versus2.json".
        static const char* LevelFile(GameMode mode, int level);

        SceneManager& m_sceneManager;
        ResourceManager& m_resourceManager;
        InputManager& m_inputManager;
        Scene& m_menuScene;
        Scene& m_gameScene;
        StartMenuComponent* m_pMenu{ nullptr };

        Pending m_pending{ Pending::None };
        GameMode m_pendingMode{ GameMode::SinglePlayer };

        // Which mode/level is on screen now (drives F1 level-skip progression).
        GameMode m_currentMode{ GameMode::SinglePlayer };
        int m_currentLevel{ 1 };
    };
}
