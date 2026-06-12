#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "Scene.h"
#include "PengoCharacter.h"
#include "SnoBeeCharacter.h"
#include "TypeRegistry.h"
#include "Achievements.h"
#include "MazeDrawingComponent.h"
#include "BorderComponent.h"
#include "LivesIconComponent.h"
#include "SnoBeeCounterComponent.h"
#include "ScoreDisplayComponent.h"
#include "HighScoreDisplayComponent.h"
#include "LevelDisplayComponent.h"
#include "GameDebugUI.h"
#include "ImGuiManager.h"

#include <filesystem>
#include <vector>

#include "ServiceLocator.h"
#include "SDLSoundSystem.h"
#include "LoggingSoundSystem.h"

namespace fs = std::filesystem;

static void load(dae::SceneManager &sceneManager, dae::ResourceManager &resourceManager, dae::InputManager &inputManager)
{
	auto &scene = sceneManager.CreateScene();
	auto canvas = std::make_unique<dae::GameObject>("Canvas");
	auto *canvasPtr = canvas.get();
	scene.Add(std::move(canvas));

	// logo
	auto go = std::make_unique<dae::GameObject>();
	go->SetName("Logo");
	go->AddComponent<dae::RenderComponent>(resourceManager)->SetTexture("logo.png");
	go->SetPosition(470, 428);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// fps counter
	auto fpsFont = resourceManager.LoadFont("Lingua.otf", 16);
	go = std::make_unique<dae::GameObject>();
	go->SetName("FPS Counter");
	go->AddComponent<dae::TextComponent>("0 FPS", fpsFont, dae::TextComponent::Color{255, 255, 255, 255});
	go->AddComponent<dae::FPSComponent>();
	go->SetPosition(500, 20);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	inputManager.ClearBindings();

	auto pengo = std::make_unique<dae::PengoCharacter>(resourceManager);
	auto *pengoPtr = pengo.get();
	pengo->BindKeyboardControls(inputManager);
	pengo->SetPosition(-1000, -1000);

	auto mazeIntro = std::make_unique<dae::GameObject>("Maze Intro");
	auto *mazeComp = mazeIntro->AddComponent<dae::MazeDrawingComponent>(scene, resourceManager, "level1.json", [pengoPtr](glm::vec2 pengoSpawnPos)
													   {
														   pengoPtr->SetPosition(pengoSpawnPos.x, pengoSpawnPos.y);
													   });
	mazeComp->SetPengo(pengoPtr); // let the maze drive the death / respawn sequence

	// Field border that rattles when Pengo pushes into a wall (and stuns Sno-Bees along it)
	auto *borderComp = mazeIntro->AddComponent<dae::BorderComponent>(scene, resourceManager);
	pengoPtr->SetBorder(borderComp);

	mazeIntro->SetPosition(0, 0);
	scene.Add(std::move(mazeIntro));

	scene.Add(std::move(pengo));

	// Lives display in the empty space to the right of the playfield
	auto lives = std::make_unique<dae::GameObject>("Lives");
	lives->AddComponent<dae::LivesIconComponent>(resourceManager, pengoPtr);
	lives->SetPosition(760, 120);
	scene.Add(std::move(lives));

	// Reserve Sno-Bee counter: stack of circles in the empty space on the right
	auto snoBeeCounter = std::make_unique<dae::GameObject>("SnoBee Counter");
	snoBeeCounter->AddComponent<dae::SnoBeeCounterComponent>(resourceManager, mazeComp);
	snoBeeCounter->SetPosition(850, 120);
	scene.Add(std::move(snoBeeCounter));

	// HUD text (PressStart2P arcade font): high score, score and level, stacked on the right
	auto hudFont = resourceManager.LoadFont("PressStart2P-Regular.ttf", 16);

	auto hiScore = std::make_unique<dae::GameObject>("HighScore");
	hiScore->AddComponent<dae::TextComponent>("HI-SCORE: 0", hudFont, dae::TextComponent::Color{255, 209, 0, 255});
	hiScore->AddComponent<dae::HighScoreDisplayComponent>(pengoPtr, "HI-SCORE");
	hiScore->SetPosition(500, 50);
	scene.Add(std::move(hiScore));

	auto scoreGo = std::make_unique<dae::GameObject>("Score");
	scoreGo->AddComponent<dae::TextComponent>("SCORE: 0", hudFont, dae::TextComponent::Color{255, 255, 255, 255});
	scoreGo->AddComponent<dae::ScoreDisplayComponent>(std::vector<dae::Character*>{pengoPtr}, std::string("SCORE"));
	scoreGo->SetPosition(500, 78);
	scene.Add(std::move(scoreGo));

	auto levelGo = std::make_unique<dae::GameObject>("Level");
	levelGo->AddComponent<dae::TextComponent>("LEVEL: 1", hudFont, dae::TextComponent::Color{255, 255, 255, 255});
	levelGo->AddComponent<dae::LevelDisplayComponent>(mazeComp, "LEVEL");
	levelGo->SetPosition(500, 106);
	scene.Add(std::move(levelGo));

	if (dae::Achievements *achievements = dae::Achievements::GetActiveInstance(); achievements != nullptr)
	{
		achievements->ObserveCharacter(pengoPtr);
	}

	dae::GameDebugUI::RegisterCustomTabs(dae::ImGuiManager::GetInstance());
}

int main(int, char *[])
{
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if (!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);

	// Audio Service Locator Setup
	// Use SDLSoundSystem for ALL platforms! We previously made it synchronous for Emscripten to avoid thread issues.
	auto sdl_ss = std::make_unique<dae::SDLSoundSystem>(data_location.string());
	dae::ServiceLocator::register_sound_system(std::make_unique<dae::LoggingSoundSystem>(std::move(sdl_ss)));

	engine.Run(load);
	return 0;
}
