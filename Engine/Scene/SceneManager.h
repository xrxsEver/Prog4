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

		// Only the active scene is updated and rendered; the first scene created becomes active.
		// This is what lets the game swap between the menu and a running game mode.
		void SetActiveScene(Scene *pScene) { m_pActiveScene = pScene; }
		Scene *GetActiveScene() const { return m_pActiveScene; }

		void Update(float deltaTime);
		void FixedUpdate();
		void Render();

	private:
		std::vector<std::unique_ptr<Scene>> m_scenes{};
		Scene *m_pActiveScene{nullptr};
	};
}
