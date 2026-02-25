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
#include "RotationComponent.h"
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

	// Sprite is 15x15, top-left of first row in the sprite sheet
	constexpr float spriteSize = 15.f;
	constexpr float displaySize = 25.f; // scale

	auto pivot = std::make_unique<dae::GameObject>();
	pivot->SetPosition(320, 240); // screen center

	auto character1 = std::make_unique<dae::GameObject>();
	auto *pengo = character1->AddComponent<dae::RenderComponent>();
	pengo->SetTexture("pengo.png");
	pengo->SetSourceRect(0.f, 0.f, spriteSize, spriteSize);
	pengo->SetRenderSize(displaySize, displaySize);
	character1->AddComponent<dae::RotationComponent>(30.f, -5.f); //CW

	auto *rawChar1 = character1.get();
	character1->SetParent(pivot.get(), false);

	// 11 rows down -> y = 10 * 15 = 165
	auto character2 = std::make_unique<dae::GameObject>();
	auto *sno = character2->AddComponent<dae::RenderComponent>();
	sno->SetTexture("pengo.png");
	sno->SetSourceRect(0.f, 10.f * spriteSize - 6, spriteSize, spriteSize);
	sno->SetRenderSize(displaySize, displaySize);
	character2->AddComponent<dae::RotationComponent>(40.f, 7.f); //CCW

	character2->SetParent(rawChar1, false);

	// here order matters: parent first
	scene.Add(std::move(pivot));
	scene.Add(std::move(character1));
	scene.Add(std::move(character2));
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
