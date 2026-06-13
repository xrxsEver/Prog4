#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"

#include "GameMode.h"
#include "GameController.h"
#include "StartMenuComponent.h"
#include "CallbackComponent.h"

#ifdef DAE_DEBUG_UI
#include "GameDebugUI.h"
#include "ImGuiManager.h"
#endif

#include <array>
#include <memory>
#include <string>
#include <filesystem>

#include "ServiceLocator.h"
#include "SDLSoundSystem.h"
#include "LoggingSoundSystem.h"

namespace fs = std::filesystem;

static void load(dae::SceneManager &sceneManager, dae::ResourceManager &resourceManager, dae::InputManager &inputManager)
{
	// Three scenes: the start menu (active first), an initially-empty scene the chosen mode is
	// built into, and a score scene for the end-of-run name entry / high-score board. The
	// GameController switches between them.
	auto &menuScene = sceneManager.CreateScene();
	auto &gameScene = sceneManager.CreateScene();
	auto &scoreScene = sceneManager.CreateScene();

	// The flow coordinator outlives any single scene; the active scene pumps its Tick().
	static std::unique_ptr<dae::GameController> s_controller;
	s_controller = std::make_unique<dae::GameController>(sceneManager, resourceManager, inputManager, menuScene, gameScene, scoreScene);
	auto *gc = s_controller.get();

	// --- Start menu UI ---
	auto menuCanvas = std::make_unique<dae::GameObject>("Menu Canvas");
	auto *menuCanvasPtr = menuCanvas.get();
	menuScene.Add(std::move(menuCanvas));

	auto logo = std::make_unique<dae::GameObject>("Menu Logo");
	logo->AddComponent<dae::RenderComponent>(resourceManager)->SetTexture("logo.png");
	logo->SetPosition(362, 90);
	logo->SetParent(menuCanvasPtr, false);
	menuScene.Add(std::move(logo));

	auto menuFont = resourceManager.LoadFont("PressStart2P-Regular.ttf", 20);
	auto smallFont = resourceManager.LoadFont("PressStart2P-Regular.ttf", 10);

	const char *names[dae::kGameModeCount] = {"SINGLE PLAYER", "CO-OP", "VERSUS"};
	std::array<dae::TextComponent *, dae::kGameModeCount> labels{};
	for (int i = 0; i < dae::kGameModeCount; ++i)
	{
		auto row = std::make_unique<dae::GameObject>("Menu Row");
		labels[i] = row->AddComponent<dae::TextComponent>(std::string("  ") + names[i], menuFont, dae::TextComponent::Color{255, 255, 255, 255});
		row->SetPosition(330.f, 250.f + static_cast<float>(i) * 44.f);
		menuScene.Add(std::move(row));
	}

	auto menuObj = std::make_unique<dae::GameObject>("Start Menu");
	auto *menuComp = menuObj->AddComponent<dae::StartMenuComponent>(inputManager, labels,
																	[gc](dae::GameMode mode)
																	{ gc->RequestMode(mode); });
	menuScene.Add(std::move(menuObj));

	gc->SetMenu(menuComp);
	gc->BindMenuInput();

	auto hint = std::make_unique<dae::GameObject>("Menu Hint");
	hint->AddComponent<dae::TextComponent>("W/S + SPACE   OR   D-PAD + A", smallFont, dae::TextComponent::Color{160, 160, 160, 255});
	hint->SetPosition(250, 470);
	menuScene.Add(std::move(hint));

	auto hint2 = std::make_unique<dae::GameObject>("Menu Hint 2");
	hint2->AddComponent<dae::TextComponent>("CONNECT A CONTROLLER FOR CO-OP / VERSUS", smallFont, dae::TextComponent::Color{160, 160, 160, 255});
	hint2->SetPosition(190, 500);
	menuScene.Add(std::move(hint2));

	// While the menu is the active scene, this pump carries out deferred mode transitions.
	auto menuPump = std::make_unique<dae::GameObject>("Flow Pump");
	menuPump->AddComponent<dae::CallbackComponent>([gc]
												   { gc->Tick(); });
	menuScene.Add(std::move(menuPump));

#ifdef DAE_DEBUG_UI
	dae::GameDebugUI::RegisterCustomTabs(dae::ImGuiManager::GetInstance());
#endif
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
