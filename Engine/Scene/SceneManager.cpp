#include "SceneManager.h"
#include "Scene.h"

void dae::SceneManager::Update(float deltaTime)
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->Update(deltaTime);
		return;
	}

	for (auto &scene : m_scenes)
	{
		scene->Update(deltaTime);
	}
}

void dae::SceneManager::FixedUpdate()
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->FixedUpdate();
		return;
	}

	for (auto &scene : m_scenes)
	{
		scene->FixedUpdate();
	}
}

void dae::SceneManager::Render()
{
	if (m_pActiveScene != nullptr)
	{
		m_pActiveScene->Render();
		return;
	}

	for (const auto &scene : m_scenes)
	{
		scene->Render();
	}
}

dae::Scene &dae::SceneManager::CreateScene()
{
	m_scenes.emplace_back(std::make_unique<Scene>());

	// First scene created drives the loop until someone switches the active scene.
	if (m_pActiveScene == nullptr)
	{
		m_pActiveScene = m_scenes.back().get();
	}

	return *m_scenes.back();
}

void dae::SceneManager::RemoveAllScenes()
{
	m_pActiveScene = nullptr;
	m_scenes.clear();
}
