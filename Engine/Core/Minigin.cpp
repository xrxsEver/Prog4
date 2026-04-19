#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <memory>
#include <string_view>

#include <SDL3/SDL.h>
// #include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Minigin.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameTime.h"
#include "ImGuiManager.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

namespace
{
	using Clock = std::chrono::high_resolution_clock;
	constexpr auto g_DesiredFrameTime = std::chrono::milliseconds(16);

	void LogSDLVersion(const std::string_view message, const int major, const int minor, const int patch)
	{
		std::cout << message << major << "." << minor << "." << patch << "\n";
	}

#ifdef __EMSCRIPTEN__
	void LoopCallback(void *arg)
	{
		static_cast<dae::Minigin *>(arg)->RunOneFrame();
	}
#endif

	// Why bother with this? Because sometimes students have a different SDL version installed on their pc.
	// That is not a problem unless for some reason the dll's from this project are not copied next to the exe.
	// These entries in the debug output help to identify that issue.
	void PrintSDLVersion()
	{
		LogSDLVersion("Compiled with SDL", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
		int version = SDL_GetVersion();
		LogSDLVersion("Linked with SDL ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
		// LogSDLVersion("Compiled with SDL_image ",SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_MICRO_VERSION);
		// version = IMG_Version();
		// LogSDLVersion("Linked with SDL_image ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
		LogSDLVersion("Compiled with SDL_ttf ", SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION, SDL_TTF_MICRO_VERSION);
		version = TTF_Version();
		LogSDLVersion("Linked with SDL_ttf ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
	}
}

class dae::Minigin::SdlRuntime final
{
public:
	SdlRuntime()
	{
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
		{
			SDL_Log("SDL init error: %s", SDL_GetError());
			throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
		}

		m_isInitialized = true;
		m_window.reset(SDL_CreateWindow(
			"Programming 4 assignment",
			1024,
			576,
			SDL_WINDOW_OPENGL));

		if (m_window == nullptr)
		{
			throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
		}
	}

	~SdlRuntime()
	{
		m_window.reset();

		if (m_isInitialized)
		{
			SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		}
	}

	SDL_Window *GetWindow() const { return m_window.get(); }

private:
	struct SDLWindowDeleter final
	{
		void operator()(SDL_Window *window) const noexcept
		{
			if (window != nullptr)
			{
				SDL_DestroyWindow(window);
			}
		}
	};

	using WindowPtr = std::unique_ptr<SDL_Window, SDLWindowDeleter>;

	WindowPtr m_window{};
	bool m_isInitialized{};
};

void dae::Minigin::RunFrame(const float deltaTime)
{
	constexpr float fixedTimeStep = GameTime::GetFixedTimeStep();

	GameTime::GetInstance().SetDeltaTime(deltaTime);
	m_quit = !m_inputManager.ProcessInput();

	m_fixedLag += deltaTime;
	while (m_fixedLag >= fixedTimeStep)
	{
		m_sceneManager.FixedUpdate();
		m_fixedLag -= fixedTimeStep;
	}

	m_achievements.Update();
	m_sceneManager.Update();

	auto &renderer = Renderer::GetInstance();
	auto &imgui = ImGuiManager::GetInstance();
	SDL_Renderer *sdlRenderer = renderer.GetSDLRenderer();

	imgui.BeginFrame();
	renderer.Render();
	m_sceneManager.Render();
	imgui.EndFrame(sdlRenderer);
	SDL_RenderPresent(sdlRenderer);
}

dae::Minigin::Minigin(const std::filesystem::path &dataPath)
{
	PrintSDLVersion();
	m_pSdlRuntime = std::make_unique<SdlRuntime>();

	Renderer::GetInstance().Init(m_pSdlRuntime->GetWindow());
	m_resourceManager.Init(dataPath);
	ImGuiManager::GetInstance().Init(m_pSdlRuntime->GetWindow(), Renderer::GetInstance().GetSDLRenderer(), m_sceneManager, m_inputManager);
}

dae::Minigin::~Minigin()
{
	ImGuiManager::GetInstance().ShutDown();
	m_sceneManager.RemoveAllScenes();
	m_resourceManager.ShutDown();
	Renderer::GetInstance().Destroy();
	m_pSdlRuntime.reset();
}

void dae::Minigin::Run(const LoadFunction &load)
{
	load(m_sceneManager, m_resourceManager, m_inputManager);
	m_fixedLag = 0.0f;

#ifndef __EMSCRIPTEN__
	auto lastTime = Clock::now();

	while (!m_quit)
	{
		const auto frameStart = Clock::now();
		const float rawDeltaTime = std::chrono::duration<float>(frameStart - lastTime).count();
		lastTime = frameStart;

		const float clampedDeltaTime = (std::min)(rawDeltaTime, GameTime::GetMaxDeltaTime());
		RunFrame(clampedDeltaTime);

		const auto frameDuration = Clock::now() - frameStart;
		if (frameDuration < g_DesiredFrameTime)
		{
			const auto sleepTime = g_DesiredFrameTime - frameDuration;
			std::this_thread::sleep_for(sleepTime);
		}
	}
#else
	emscripten_set_main_loop_arg(&LoopCallback, this, 0, true);
#endif
}

void dae::Minigin::RunOneFrame()
{
	static auto lastTime = Clock::now();
	const auto currentTime = Clock::now();
	const float rawDeltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	lastTime = currentTime;

	const float clampedDeltaTime = (std::min)(rawDeltaTime, GameTime::GetMaxDeltaTime());
	RunFrame(clampedDeltaTime);

#ifdef __EMSCRIPTEN__
	if (m_quit)
	{
		emscripten_cancel_main_loop();
	}
#endif
}
