#include <SDL3/SDL.h>
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
#include "RotationComponent.h"
#include "Scene.h"
#include "PengoCharacter.h"
#include "RemainingLivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"
#include "SnoBeeCharacter.h"
#include "Achievements.h"

#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

static void load(dae::SceneManager &sceneManager, dae::ResourceManager &resourceManager, dae::InputManager &inputManager)
{
	auto &scene = sceneManager.CreateScene();
	auto canvas = std::make_unique<dae::GameObject>("Canvas");
	auto *canvasPtr = canvas.get();
	scene.Add(std::move(canvas));

	// background
	auto go = std::make_unique<dae::GameObject>();
	go->SetName("Background");
	go->AddComponent<dae::RenderComponent>(resourceManager)->SetTexture("background.png");
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// logo
	go = std::make_unique<dae::GameObject>();
	go->SetName("Logo");
	go->AddComponent<dae::RenderComponent>(resourceManager)->SetTexture("logo.png");
	go->SetPosition(10, 288);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// // title text
	// auto font = resourceManager.LoadFont("Lingua.otf", 36);
	// go = std::make_unique<dae::GameObject>();
	// go->SetName("Title");
	// go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{255, 255, 0, 255});
	// go->SetPosition(292, 20);
	// go->SetParent(canvasPtr, false);
	// scene.Add(std::move(go));

	// fps counter
	auto fpsFont = resourceManager.LoadFont("Lingua.otf", 22);
	go = std::make_unique<dae::GameObject>();
	go->SetName("FPS Counter");
	go->AddComponent<dae::TextComponent>("0 FPS", fpsFont, SDL_Color{255, 255, 255, 255});
	go->AddComponent<dae::FPSComponent>();
	go->SetPosition(20, 20);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	inputManager.ClearBindings();

	auto pengo = std::make_unique<dae::PengoCharacter>(resourceManager);
	auto *pengoPtr = pengo.get();
	pengo->SetPosition(384, 288);
	pengo->BindKeyboardControls(inputManager);
	scene.Add(std::move(pengo));

	auto snoBee = std::make_unique<dae::SnoBeeCharacter>(resourceManager);
	auto *snoBeePtr = snoBee.get();
	snoBee->SetPosition(576, 288);
	snoBee->BindGamepadControls(inputManager, dae::InputManager::AnyGamepad);
	scene.Add(std::move(snoBee));

	if (dae::Achievements *achievements = dae::Achievements::GetActiveInstance(); achievements != nullptr)
	{
		achievements->ObserveCharacter(pengoPtr);
		achievements->ObserveCharacter(snoBeePtr);
	}

	auto pengoLivesDisplay = std::make_unique<dae::GameObject>("Pengo Lives");
	pengoLivesDisplay->AddComponent<dae::TextComponent>("Pengo lives: 3", fpsFont, SDL_Color{255, 255, 255, 255});
	pengoLivesDisplay->AddComponent<dae::RemainingLivesDisplayComponent>(pengoPtr, "Pengo lives");
	pengoLivesDisplay->SetPosition(20, 52);
	pengoLivesDisplay->SetParent(canvasPtr, false);
	scene.Add(std::move(pengoLivesDisplay));

	auto snoBeeLivesDisplay = std::make_unique<dae::GameObject>("SnoBee Lives");
	snoBeeLivesDisplay->AddComponent<dae::TextComponent>("SnoBee lives: 3", fpsFont, SDL_Color{255, 255, 255, 255});
	snoBeeLivesDisplay->AddComponent<dae::RemainingLivesDisplayComponent>(snoBeePtr, "SnoBee lives");
	snoBeeLivesDisplay->SetPosition(20, 84);
	snoBeeLivesDisplay->SetParent(canvasPtr, false);
	scene.Add(std::move(snoBeeLivesDisplay));

	auto pengoPointsDisplay = std::make_unique<dae::GameObject>("Pengo Points");
	pengoPointsDisplay->AddComponent<dae::TextComponent>("Pengo points: 0", fpsFont, SDL_Color{255, 255, 255, 255});
	pengoPointsDisplay->AddComponent<dae::ScoreDisplayComponent>(std::vector<dae::Character *>{pengoPtr}, "Pengo points");
	pengoPointsDisplay->SetPosition(20, 116);
	pengoPointsDisplay->SetParent(canvasPtr, false);
	scene.Add(std::move(pengoPointsDisplay));

	auto snoBeePointsDisplay = std::make_unique<dae::GameObject>("SnoBee Points");
	snoBeePointsDisplay->AddComponent<dae::TextComponent>("SnoBee points: 0", fpsFont, SDL_Color{255, 255, 255, 255});
	snoBeePointsDisplay->AddComponent<dae::ScoreDisplayComponent>(std::vector<dae::Character *>{snoBeePtr}, "SnoBee points");
	snoBeePointsDisplay->SetPosition(20, 148);
	snoBeePointsDisplay->SetParent(canvasPtr, false);
	scene.Add(std::move(snoBeePointsDisplay));

	auto controlsHintKeyboard = std::make_unique<dae::GameObject>("Controls Hint Keyboard");
	controlsHintKeyboard->AddComponent<dae::TextComponent>("Pengo : X +10 points | V +100 points | C lose life", fpsFont, SDL_Color{210, 220, 235, 255});
	controlsHintKeyboard->SetPosition(20, 520);
	controlsHintKeyboard->SetParent(canvasPtr, false);
	scene.Add(std::move(controlsHintKeyboard));

	auto controlsHintGamepad = std::make_unique<dae::GameObject>("Controls Hint Gamepad");
	controlsHintGamepad->AddComponent<dae::TextComponent>("SnoBee : A +10 points | B +100 points | X lose life", fpsFont, SDL_Color{210, 220, 235, 255});
	controlsHintGamepad->SetPosition(20, 548);
	controlsHintGamepad->SetParent(canvasPtr, false);
	scene.Add(std::move(controlsHintGamepad));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
	return 0;
}
