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
#include "Scene.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto &scene = dae::SceneManager::GetInstance().CreateScene();

	// background
	auto go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>()->SetTexture("background.png");
	scene.Add(std::move(go));

	// logo
	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::RenderComponent>()->SetTexture("logo.png");
	go->SetPosition(358, 180);
	scene.Add(std::move(go));

	// title text
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{255, 255, 0, 255});
	go->SetPosition(292, 20);
	scene.Add(std::move(go));

	// fps counter
	auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);
	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::TextComponent>("0 FPS", fpsFont, SDL_Color{255, 255, 255, 255});
	go->AddComponent<dae::FPSComponent>();
	go->SetPosition(20, 20);
	scene.Add(std::move(go));
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
