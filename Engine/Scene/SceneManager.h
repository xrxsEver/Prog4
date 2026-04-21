#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Scene.h"

namespace dae
{
	class Scene;
	class SceneManager final
	{
	public:
		SceneManager() = default;
		Scene &CreateScene();
		const std::vector<std::unique_ptr<Scene>> &GetScenes() const { return m_scenes; }
		void RemoveAllScenes();

		void Update(float deltaTime);
		void FixedUpdate();
		void Render();

	private:
		std::vector<std::unique_ptr<Scene>> m_scenes{};
	};
}
