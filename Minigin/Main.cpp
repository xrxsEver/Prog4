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

#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

static void load()
{
	auto &scene = dae::SceneManager::GetInstance().CreateScene();
	auto canvas = std::make_unique<dae::GameObject>("Canvas");
	auto *canvasPtr = canvas.get();
	scene.Add(std::move(canvas));

	// background
	auto go = std::make_unique<dae::GameObject>();
	go->SetName("Background");
	go->AddComponent<dae::RenderComponent>()->SetTexture("background.png");
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// logo
	go = std::make_unique<dae::GameObject>();
	go->SetName("Logo");
	go->AddComponent<dae::RenderComponent>()->SetTexture("logo.png");
	go->SetPosition(358, 180);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// title text
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	go = std::make_unique<dae::GameObject>();
	go->SetName("Title");
	go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{255, 255, 0, 255});
	go->SetPosition(292, 20);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	// fps counter
	auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);
	go = std::make_unique<dae::GameObject>();
	go->SetName("FPS Counter");
	go->AddComponent<dae::TextComponent>("0 FPS", fpsFont, SDL_Color{255, 255, 255, 255});
	go->AddComponent<dae::FPSComponent>();
	go->SetPosition(20, 20);
	go->SetParent(canvasPtr, false);
	scene.Add(std::move(go));

	auto &input = dae::InputManager::GetInstance();
	input.ClearBindings();

	auto pengo = std::make_unique<dae::PengoCharacter>();
	auto *pengoPtr = pengo.get();
	pengo->SetPosition(384, 288);
	pengo->BindKeyboardControls();
	scene.Add(std::move(pengo));

	auto snoBee = std::make_unique<dae::SnoBeeCharacter>();
	auto *snoBeePtr = snoBee.get();
	snoBee->SetPosition(576, 288);
	snoBee->BindGamepadControls(dae::InputManager::AnyGamepad);
	scene.Add(std::move(snoBee));

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
