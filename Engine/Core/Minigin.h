#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include <memory>

#include "Achievements.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
		Achievements m_achievements{};
		InputManager m_inputManager{};
		SceneManager m_sceneManager{};
		ResourceManager m_resourceManager{};
		class SdlRuntime;
		std::unique_ptr<SdlRuntime> m_pSdlRuntime{};
		float m_fixedLag{};

		void RunFrame(float deltaTime);

	public:
		using LoadFunction = std::function<void(SceneManager &, ResourceManager &, InputManager &)>;

		explicit Minigin(const std::filesystem::path &dataPath);
		~Minigin();
		void Run(const LoadFunction &load);
		void RunOneFrame();

		Minigin(const Minigin &other) = delete;
		Minigin(Minigin &&other) = delete;
		Minigin &operator=(const Minigin &other) = delete;
		Minigin &operator=(Minigin &&other) = delete;
	};
}