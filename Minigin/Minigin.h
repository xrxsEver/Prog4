#pragma once
#include <string>
#include <functional>
#include <filesystem>

#include "Achievements.h"

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
		Achievements m_achievements{};

	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}