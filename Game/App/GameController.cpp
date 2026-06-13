#include "GameController.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "SceneManager.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"

#include "RenderComponent.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "CallbackComponent.h"

#include "PengoCharacter.h"
#include "MazeDrawingComponent.h"
#include "BorderComponent.h"
#include "LivesIconComponent.h"
#include "SnoBeeCounterComponent.h"
#include "ScoreDisplayComponent.h"
#include "HighScoreDisplayComponent.h"
#include "LevelDisplayComponent.h"
#include "TimeDisplayComponent.h"

#include "StartMenuComponent.h"
#include "VersusControllerComponent.h"
#include "NameEntryComponent.h"
#include "MenuCommands.h"
#include "CallbackCommand.h"

#include "Character.h"
#include "HighScores.h"

namespace dae
{
    GameController::GameController(SceneManager& sceneManager, ResourceManager& resourceManager,
                                  InputManager& inputManager, Scene& menuScene, Scene& gameScene, Scene& scoreScene)
        : m_sceneManager(sceneManager)
        , m_resourceManager(resourceManager)
        , m_inputManager(inputManager)
        , m_menuScene(menuScene)
        , m_gameScene(gameScene)
        , m_scoreScene(scoreScene)
    {
    }

    void GameController::RequestMode(GameMode mode)
    {
        m_pending = Pending::EnterMode;
        m_pendingMode = mode;
    }

    void GameController::RequestNextLevel()
    {
        m_pending = Pending::NextLevel;
    }

    void GameController::RequestMenu()
    {
        m_pending = Pending::ShowMenu;
    }

    void GameController::RequestEndRun(bool completed)
    {
        m_pending = Pending::EndRun;
        m_pendingCompleted = completed;
    }

    void GameController::RequestShowBoard(const std::string& name, int score)
    {
        m_pending = Pending::ShowBoard;
        m_boardHighlightName = name;
        m_boardScore = score;
    }

    void GameController::Tick()
    {
        // Clear the request before acting so a transition can safely queue another one.
        const Pending pending = m_pending;
        m_pending = Pending::None;

        switch (pending)
        {
            // EnterMode/NextLevel/EndRun tear the current scene down and rebuild. When the pump that
            // runs this lives in the very scene being rebuilt (a level skip / clear / death), doing it
            // now would clear the object list mid-iteration — so defer to the end of the active update.
        case Pending::EnterMode:  { const GameMode mode = m_pendingMode; ScheduleRebuild([this, mode] { EnterMode(mode); }); break; }
        case Pending::NextLevel:  ScheduleRebuild([this] { NextLevel(); }); break;
        case Pending::EndRun:     { const bool c = m_pendingCompleted; ScheduleRebuild([this, c] { EndRun(c); }); break; }
        case Pending::ShowBoard:  { const std::string n = m_boardHighlightName; const int s = m_boardScore; ScheduleRebuild([this, n, s] { BuildScoreBoardScene(n, s); }); break; }
        case Pending::ShowMenu:   ShowMenu(); break; // just swaps the active scene; safe immediately
        case Pending::None:       break;
        }
    }

    void GameController::ScheduleRebuild(std::function<void()> action)
    {
        if (Scene* active = m_sceneManager.GetActiveScene())
        {
            active->RunAfterUpdate(std::move(action));
        }
        else
        {
            action();
        }
    }

    void GameController::BindMenuInput()
    {
        if (m_pMenu == nullptr)
        {
            return;
        }

        // Keyboard: W/S move, Space selects.
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Down, std::make_unique<MenuNavigateCommand>(*m_pMenu, -1));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Down, std::make_unique<MenuNavigateCommand>(*m_pMenu, 1));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_SPACE, KeyState::Down, std::make_unique<MenuConfirmCommand>(*m_pMenu));

        // Any connected gamepad can drive the menu too: D-pad moves, A selects.
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadUp, KeyState::Down, std::make_unique<MenuNavigateCommand>(*m_pMenu, -1));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadDown, KeyState::Down, std::make_unique<MenuNavigateCommand>(*m_pMenu, 1));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::A, KeyState::Down, std::make_unique<MenuConfirmCommand>(*m_pMenu));

        BindMuteToggle();
    }

    void GameController::BindMuteToggle()
    {
        // F2 flips master mute. The mute state lives in the sound system (a global service), so it
        // persists across scene rebuilds — re-binding here just re-attaches the key each scene.
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_F2, KeyState::Down,
            std::make_unique<CallbackCommand>([]
            {
                auto& sound = ServiceLocator::get_sound_system();
                sound.set_muted(!sound.is_muted());
            }));
    }

    void GameController::ShowMenu()
    {
        // No level music on the menu.
        ServiceLocator::get_sound_system().stop_music();

        m_inputManager.ClearBindings();
        BindMenuInput();
        if (m_pMenu != nullptr)
        {
            m_pMenu->RefreshNow();
        }
        m_sceneManager.SetActiveScene(&m_menuScene);
    }

    void GameController::EnterMode(GameMode mode)
    {
        // A fresh run: zero the score and fill the life pool, then start at the first level.
        m_runScore = 0;
        m_runLives = kStartLives;
        LoadLevel(mode, 1);
    }

    void GameController::NextLevel()
    {
        // Carry this level's score and remaining lives into the run before rebuilding (the players
        // are recreated per level, so the totals live on the controller between them).
        m_runScore = CaptureScore();
        m_runLives = CaptureLives();

        // Step to the next level, or — once the last level is cleared — end the run as "completed"
        // so the player gets to enter their name. (We're already inside the deferred rebuild here,
        // so calling EndRun directly is safe: the game scene's objects haven't been torn down yet.)
        const int next = m_currentLevel + 1;
        if (next > kLevelsPerMode)
        {
            EndRun(true);
            return;
        }
        LoadLevel(m_currentMode, next);
    }

    void GameController::LoadLevel(GameMode mode, int level)
    {
        m_currentMode = mode;
        m_currentLevel = level;

        m_inputManager.ClearBindings();

        // Clear the previous scene first (its grid objects unregister from the live grid), then
        // swap in a fresh, empty collision grid for the new level.
        m_gameScene.RemoveAll();
        ServiceLocator::register_collision_grid(nullptr);

        BuildGameScene(mode, level);

        m_sceneManager.SetActiveScene(&m_gameScene);
    }

    const char* GameController::LevelFile(GameMode mode, int level)
    {
        // Base name only (no extension); MazeGenerator::Load picks .json (debug) or .bin (release).
        static const char* kSingle[kLevelsPerMode]  = { "single1", "single2", "single3" };
        static const char* kCoop[kLevelsPerMode]    = { "coop1",   "coop2",   "coop3"   };
        static const char* kVersus[kLevelsPerMode]  = { "versus1", "versus2", "versus3" };

        const int i = std::min(std::max(level, 1), kLevelsPerMode) - 1;
        switch (mode)
        {
        case GameMode::CoOp:   return kCoop[i];
        case GameMode::Versus: return kVersus[i];
        case GameMode::SinglePlayer:
        default:               return kSingle[i];
        }
    }

    void GameController::BindReturnToMenu()
    {
        // Escape (keyboard) or Start (any gamepad) drops back to the start menu.
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_ESCAPE, KeyState::Down,
            std::make_unique<CallbackCommand>([this] { RequestMenu(); }));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::Start, KeyState::Down,
            std::make_unique<CallbackCommand>([this] { RequestMenu(); }));

        // F1 skips to the next level (and back to the menu after the last one).
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_F1, KeyState::Down,
            std::make_unique<CallbackCommand>([this] { RequestNextLevel(); }));

        BindMuteToggle();
    }

    void GameController::BuildGameScene(GameMode mode, int level)
    {
        Scene& scene = m_gameScene;

        const bool coop = (mode == GameMode::CoOp);
        const bool versus = (mode == GameMode::Versus);
        const bool twoPlayers = coop || versus;
        const char* levelFile = LevelFile(mode, level);

        // --- Shared HUD furniture (logo, FPS) ---
        auto canvas = std::make_unique<GameObject>("Canvas");
        auto* canvasPtr = canvas.get();
        scene.Add(std::move(canvas));

        auto logo = std::make_unique<GameObject>("Logo");
        logo->AddComponent<RenderComponent>(m_resourceManager)->SetTexture("logo.png");
        logo->SetPosition(470, 428);
        logo->SetParent(canvasPtr, false);
        scene.Add(std::move(logo));

        auto fpsFont = m_resourceManager.LoadFont("Lingua.otf", 16);
        auto fps = std::make_unique<GameObject>("FPS Counter");
        fps->AddComponent<TextComponent>("0 FPS", fpsFont, TextComponent::Color{ 255, 255, 255, 255 });
        fps->AddComponent<FPSComponent>();
        fps->SetPosition(500, 20);
        fps->SetParent(canvasPtr, false);
        scene.Add(std::move(fps));

        // --- Automatic input policy: assign devices to players based on what's connected ---
        std::vector<std::uint32_t> pads;
        for (std::uint32_t i = 0; i < 4; ++i)
        {
            if (m_inputManager.IsGamepadConnected(i))
            {
                pads.push_back(i);
            }
        }

        // Player one defaults to keyboard + the first pad (single player can use either).
        PengoControls p1Controls{ true, !pads.empty(), pads.empty() ? 0u : pads[0] };
        PengoControls p2Controls{};
        if (twoPlayers)
        {
            if (pads.size() >= 2)
            {
                p1Controls = { true, true, pads[0] };   // keyboard + pad one
                p2Controls = { false, true, pads[1] };  // second pad
            }
            else if (pads.size() == 1)
            {
                p1Controls = { true, false, 0 };         // keyboard only
                p2Controls = { false, true, pads[0] };   // the single pad
            }
            else
            {
                p2Controls = { false, true, 0 };         // no pad (menu shouldn't allow this); P2 inert
            }
        }

        // --- Player one (always a Pengo) ---
        auto pengo = std::make_unique<PengoCharacter>(m_resourceManager);
        auto* p1 = pengo.get();
        p1->BindControls(m_inputManager, p1Controls);
        pengo->SetPosition(-1000, -1000);

        auto mazeIntro = std::make_unique<GameObject>("Maze Intro");
        auto* mazeComp = mazeIntro->AddComponent<MazeDrawingComponent>(scene, m_resourceManager, levelFile,
            [p1](glm::vec2 spawnPos) { p1->SetPosition(spawnPos.x, spawnPos.y); });
        mazeComp->SetPengo(p1);

        // Clearing the board (every Sno-Bee dead) advances to the next level, just like F1.
        mazeComp->SetOnLevelComplete([this] { RequestNextLevel(); });

        // Losing the last life ends the run and sends the player to name entry.
        mazeComp->SetOnGameOver([this] { RequestEndRun(false); });

        // Field border that rattles when a Pengo pushes into a wall (and stuns Sno-Bees along it).
        auto* borderComp = mazeIntro->AddComponent<BorderComponent>(scene, m_resourceManager);
        p1->SetBorder(borderComp);

        mazeIntro->SetPosition(0, 0);
        scene.Add(std::move(mazeIntro));

        // --- Player two: co-op spawns a second Pengo; versus hijacks an AI Sno-Bee (see below) ---
        // pengo2 is kept in a local and added to the scene LAST (see note below).
        std::unique_ptr<PengoCharacter> pengo2;
        PengoCharacter* p2 = nullptr;

        if (coop)
        {
            pengo2 = std::make_unique<PengoCharacter>(m_resourceManager);
            p2 = pengo2.get();
            p2->BindControls(m_inputManager, p2Controls);
            p2->SetSpriteRowOffset(4); // player two is the orange Pengo (sprite set starts at row 4)
            p2->SetPosition(-1000, -1000);
            p2->SetBorder(borderComp);
            mazeComp->SetSecondPengo(p2);
        }
        else if (versus)
        {
            // Versus plays exactly like single player — the full AI swarm hatches — but player two
            // hijacks one Sno-Bee at a time. The controller marks it with a ring and re-targets a
            // living Sno-Bee whenever Pengo squashes the current one, so player two is never benched.
            auto versusGo = std::make_unique<GameObject>("Versus Controller");
            versusGo->AddComponent<VersusControllerComponent>(scene, m_inputManager, p2Controls.gamepadIndex);
            scene.Add(std::move(versusGo));
        }

        // --- HUD (these observe the players, so add them before the players) ---
        auto lives = std::make_unique<GameObject>("Lives");
        lives->AddComponent<LivesIconComponent>(m_resourceManager, p1);
        lives->SetPosition(760, 120);
        scene.Add(std::move(lives));

        if (p2 != nullptr)
        {
            auto lives2 = std::make_unique<GameObject>("Lives P2");
            lives2->AddComponent<LivesIconComponent>(m_resourceManager, p2);
            lives2->SetPosition(760, 160);
            scene.Add(std::move(lives2));
        }

        auto snoBeeCounter = std::make_unique<GameObject>("SnoBee Counter");
        snoBeeCounter->AddComponent<SnoBeeCounterComponent>(m_resourceManager, mazeComp);
        snoBeeCounter->SetPosition(850, 120);
        scene.Add(std::move(snoBeeCounter));

        auto hudFont = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 16);

        // Co-op shares one team score (the display sums every observed character).
        std::vector<Character*> scoreChars{ p1 };
        if (p2 != nullptr)
        {
            scoreChars.push_back(p2);
        }
        // Remember them so the end-of-run flow can read the final score before this scene is cleared.
        m_scoreChars = scoreChars;

        // Seed this level's players from the run totals so score accumulates and lives carry over.
        // Co-op shares the score (kept whole on player one; the display sums both) and the life pool.
        p1->score = m_runScore;
        p1->health = m_runLives;
        if (p2 != nullptr)
        {
            p2->score = 0;
            p2->health = m_runLives;
        }

        auto hiScore = std::make_unique<GameObject>("HighScore");
        hiScore->AddComponent<TextComponent>("HI-SCORE: 0", hudFont, TextComponent::Color{ 255, 209, 0, 255 });
        hiScore->AddComponent<HighScoreDisplayComponent>(p1, "HI-SCORE");
        hiScore->SetPosition(500, 50);
        scene.Add(std::move(hiScore));

        auto scoreGo = std::make_unique<GameObject>("Score");
        scoreGo->AddComponent<TextComponent>("SCORE: 0", hudFont, TextComponent::Color{ 255, 255, 255, 255 });
        scoreGo->AddComponent<ScoreDisplayComponent>(scoreChars, std::string("SCORE"));
        scoreGo->SetPosition(500, 78);
        scene.Add(std::move(scoreGo));

        auto levelGo = std::make_unique<GameObject>("Level");
        levelGo->AddComponent<TextComponent>("LEVEL: 1", hudFont, TextComponent::Color{ 255, 255, 255, 255 });
        levelGo->AddComponent<LevelDisplayComponent>(mazeComp, "LEVEL");
        levelGo->SetPosition(500, 106);
        scene.Add(std::move(levelGo));

        auto timeGo = std::make_unique<GameObject>("Time");
        timeGo->AddComponent<TextComponent>("TIME 0:00", hudFont, TextComponent::Color{ 255, 255, 255, 255 });
        timeGo->AddComponent<TimeDisplayComponent>(mazeComp, "TIME");
        timeGo->SetPosition(500, 134);
        scene.Add(std::move(timeGo));

        // Players (Subjects) are added LAST. Scenes destroy objects front-to-back, and a Subject
        // does not notify its observers when it dies, so the score/lives observers added above
        // must be torn down before the players they point at (otherwise: use-after-free on replay).
        scene.Add(std::move(pengo));
        if (pengo2 != nullptr)
        {
            scene.Add(std::move(pengo2));
        }

        // Return-to-menu input, and the per-frame pump that carries out deferred transitions.
        BindReturnToMenu();

        auto pump = std::make_unique<GameObject>("Flow Pump");
        pump->AddComponent<CallbackComponent>([this] { Tick(); });
        scene.Add(std::move(pump));
    }

    int GameController::CaptureScore() const
    {
        int total = 0;
        for (const Character* c : m_scoreChars)
        {
            if (c) total += c->score;
        }
        return total;
    }

    int GameController::CaptureLives() const
    {
        // Player one carries the shared life pool (co-op's two Pengos lose a life together).
        if (!m_scoreChars.empty() && m_scoreChars.front())
        {
            return m_scoreChars.front()->health;
        }
        return m_runLives;
    }

    void GameController::EndRun(bool completed)
    {
        // Read the final score off the live players BEFORE the game scene is cleared.
        const int score = CaptureScore();

        // The level is over: silence the looping theme before the score/name-entry screens.
        ServiceLocator::get_sound_system().stop_music();

        m_inputManager.ClearBindings();
        m_gameScene.RemoveAll();
        ServiceLocator::register_collision_grid(nullptr);
        m_scoreChars.clear();

        // Only prompt for initials when the score actually earns a spot; otherwise show the board.
        if (HighScores::Qualifies(score))
        {
            BuildNameEntryScene(score, completed);
        }
        else
        {
            BuildScoreBoardScene("", score);
        }
    }

    void GameController::BuildNameEntryScene(int score, bool completed)
    {
        Scene& scene = m_scoreScene;
        scene.RemoveAll();

        auto titleFont = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 20);
        auto bodyFont  = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 16);
        auto slotFont  = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 28);
        auto hintFont  = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 10);

        constexpr TextComponent::Color kWhite{ 255, 255, 255, 255 };
        constexpr TextComponent::Color kYellow{ 255, 209, 0, 255 };
        constexpr TextComponent::Color kGrey{ 160, 160, 160, 255 };

        auto title = std::make_unique<GameObject>("Title");
        title->AddComponent<TextComponent>(completed ? "CONGRATULATIONS!" : "GAME OVER",
            titleFont, completed ? kYellow : kWhite);
        title->SetPosition(completed ? 352.f : 420.f, 110.f);
        scene.Add(std::move(title));

        auto scoreLine = std::make_unique<GameObject>("Score Line");
        scoreLine->AddComponent<TextComponent>("YOUR SCORE  " + std::to_string(score), bodyFont, kWhite);
        scoreLine->SetPosition(360.f, 180.f);
        scene.Add(std::move(scoreLine));

        auto prompt = std::make_unique<GameObject>("Prompt");
        prompt->AddComponent<TextComponent>("ENTER YOUR NAME", bodyFont, kYellow);
        prompt->SetPosition(392.f, 240.f);
        scene.Add(std::move(prompt));

        // Three letter slots, spaced out and centred; the editor component drives their text/colour.
        std::array<TextComponent*, HighScores::kNameLength> slots{};
        constexpr float kSlotX0 = 454.f;
        constexpr float kSlotDX = 44.f;
        for (int i = 0; i < HighScores::kNameLength; ++i)
        {
            auto slot = std::make_unique<GameObject>("Name Slot");
            slots[i] = slot->AddComponent<TextComponent>("A", slotFont, kWhite);
            slot->SetPosition(kSlotX0 + static_cast<float>(i) * kSlotDX, 290.f);
            scene.Add(std::move(slot));
        }

        // Submit commits the row and saves the file, then drops to the board with it highlighted.
        auto onSubmit = [this, score](const std::string& name)
        {
            HighScores::Insert(name, score);
            RequestShowBoard(name, score);
        };

        auto entryObj = std::make_unique<GameObject>("Name Entry");
        auto* entry = entryObj->AddComponent<NameEntryComponent>(slots, onSubmit);
        scene.Add(std::move(entryObj));

        auto hint = std::make_unique<GameObject>("Hint");
        hint->AddComponent<TextComponent>("UP/DOWN: LETTER    LEFT/RIGHT: SLOT    SPACE/A: OK",
            hintFont, kGrey);
        hint->SetPosition(230.f, 430.f);
        scene.Add(std::move(hint));

        BindNameEntryInput(entry);

        auto pump = std::make_unique<GameObject>("Flow Pump");
        pump->AddComponent<CallbackComponent>([this] { Tick(); });
        scene.Add(std::move(pump));

        m_sceneManager.SetActiveScene(&m_scoreScene);
    }

    void GameController::BuildScoreBoardScene(const std::string& highlightName, int score)
    {
        Scene& scene = m_scoreScene;
        scene.RemoveAll();

        auto titleFont = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 20);
        auto rowFont   = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 16);
        auto hintFont  = m_resourceManager.LoadFont("PressStart2P-Regular.ttf", 10);

        constexpr TextComponent::Color kWhite{ 255, 255, 255, 255 };
        constexpr TextComponent::Color kYellow{ 255, 209, 0, 255 };
        constexpr TextComponent::Color kGrey{ 160, 160, 160, 255 };

        auto title = std::make_unique<GameObject>("Title");
        title->AddComponent<TextComponent>("HIGH SCORES", titleFont, kYellow);
        title->SetPosition(402.f, 60.f);
        scene.Add(std::move(title));

        auto scoreLine = std::make_unique<GameObject>("Score Line");
        scoreLine->AddComponent<TextComponent>("YOUR SCORE  " + std::to_string(score), rowFont, kWhite);
        scoreLine->SetPosition(360.f, 110.f);
        scene.Add(std::move(scoreLine));

        const auto entries = HighScores::Load();
        if (entries.empty())
        {
            auto empty = std::make_unique<GameObject>("Empty");
            empty->AddComponent<TextComponent>("NO SCORES YET", rowFont, kGrey);
            empty->SetPosition(380.f, 200.f);
            scene.Add(std::move(empty));
        }
        else
        {
            bool highlighted = false;
            float y = 170.f;
            for (std::size_t i = 0; i < entries.size(); ++i)
            {
                const auto& e = entries[i];
                const std::string row = std::to_string(i + 1) + ".  " + e.name + "   " + std::to_string(e.score);

                // Light up the row we just committed (first exact match only).
                const bool mine = !highlighted && !highlightName.empty()
                    && e.name == highlightName && e.score == score;
                if (mine) highlighted = true;

                auto rowObj = std::make_unique<GameObject>("Row");
                rowObj->AddComponent<TextComponent>(row, rowFont, mine ? kYellow : kWhite);
                rowObj->SetPosition(360.f, y);
                scene.Add(std::move(rowObj));
                y += 30.f;
            }
        }

        auto hint = std::make_unique<GameObject>("Hint");
        hint->AddComponent<TextComponent>("PRESS SPACE / A TO CONTINUE", hintFont, kGrey);
        hint->SetPosition(330.f, 520.f);
        scene.Add(std::move(hint));

        BindBoardInput();

        auto pump = std::make_unique<GameObject>("Flow Pump");
        pump->AddComponent<CallbackComponent>([this] { Tick(); });
        scene.Add(std::move(pump));

        m_sceneManager.SetActiveScene(&m_scoreScene);
    }

    void GameController::BindNameEntryInput(NameEntryComponent* entry)
    {
        m_inputManager.ClearBindings();

        // Up/Down scroll the current letter; Left/Right move between the three slots. Both the
        // arrow keys and WASD work, plus the D-pad, so any controller or keyboard can enter a name.
        const auto changeUp   = [entry] { entry->ChangeLetter(1); };
        const auto changeDown = [entry] { entry->ChangeLetter(-1); };
        const auto moveLeft   = [entry] { entry->MoveCursor(-1); };
        const auto moveRight  = [entry] { entry->MoveCursor(1); };
        const auto confirm    = [entry] { entry->Confirm(); };

        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_UP, KeyState::Down, std::make_unique<CallbackCommand>(changeUp));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Down, std::make_unique<CallbackCommand>(changeUp));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_DOWN, KeyState::Down, std::make_unique<CallbackCommand>(changeDown));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Down, std::make_unique<CallbackCommand>(changeDown));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_LEFT, KeyState::Down, std::make_unique<CallbackCommand>(moveLeft));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Down, std::make_unique<CallbackCommand>(moveLeft));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_RIGHT, KeyState::Down, std::make_unique<CallbackCommand>(moveRight));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Down, std::make_unique<CallbackCommand>(moveRight));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_SPACE, KeyState::Down, std::make_unique<CallbackCommand>(confirm));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_RETURN, KeyState::Down, std::make_unique<CallbackCommand>(confirm));

        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadUp, KeyState::Down, std::make_unique<CallbackCommand>(changeUp));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadDown, KeyState::Down, std::make_unique<CallbackCommand>(changeDown));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadLeft, KeyState::Down, std::make_unique<CallbackCommand>(moveLeft));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::DPadRight, KeyState::Down, std::make_unique<CallbackCommand>(moveRight));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::A, KeyState::Down, std::make_unique<CallbackCommand>(confirm));

        BindMuteToggle();
    }

    void GameController::BindBoardInput()
    {
        m_inputManager.ClearBindings();

        const auto toMenu = [this] { RequestMenu(); };
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_SPACE, KeyState::Down, std::make_unique<CallbackCommand>(toMenu));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_RETURN, KeyState::Down, std::make_unique<CallbackCommand>(toMenu));
        m_inputManager.BindKeyboardCommand(SDL_SCANCODE_ESCAPE, KeyState::Down, std::make_unique<CallbackCommand>(toMenu));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::A, KeyState::Down, std::make_unique<CallbackCommand>(toMenu));
        m_inputManager.BindGamepadCommand(InputManager::AnyGamepad, Gamepad::Button::Start, KeyState::Down, std::make_unique<CallbackCommand>(toMenu));

        BindMuteToggle();
    }
}
