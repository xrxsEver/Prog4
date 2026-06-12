#include <algorithm>
#include <cassert>
#include "Scene.h"
#include "../../Game/Characters/SnoBeeCharacter.h"
#include "../../Game/Systems/CollisionGrid.h"
#include "../../Engine/Audio/ServiceLocator.h"

using namespace dae;

Scene::Scene()
    : m_snoBeeManager(std::make_unique<SnoBeeManager>())
{
}

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objectsToAdd.emplace_back(std::move(object));
}

void Scene::AddSnoBee(SnoBeeCharacter* snoBee)
{
    m_snoBeeManager->AddSnoBee(snoBee);
}

void Scene::RunAfterUpdate(std::function<void()> action)
{
    m_afterUpdate.emplace_back(std::move(action));
}

void Scene::Remove(const GameObject &object)
{
	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[&object](const auto &ptr)
			{ return ptr.get() == &object; }),
		m_objects.end());
}

void Scene::RemoveAll()
{
	// Full reset so the scene can be rebuilt for a new game mode without stale state.
	m_objects.clear();
	m_objectsToAdd.clear();
	m_snoBeeManager = std::make_unique<SnoBeeManager>();
}

void Scene::Update(float deltaTime)
{
	if (!m_objectsToAdd.empty())
	{
		for (auto& object : m_objectsToAdd)
		{
			m_objects.emplace_back(std::move(object));
		}
		m_objectsToAdd.clear();
	}

    m_snoBeeManager->Update(deltaTime);

	for (auto &object : m_objects)
	{
        if (dynamic_cast<SnoBeeCharacter*>(object.get()) == nullptr)
        {
		    object->Update(deltaTime);
        }
	}

	// Sync the Sno-Bee manager's raw pointers before we free the objects they point at
	m_snoBeeManager->RemoveMarked();

	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[](const auto &object)
			{ return object->IsMarkedForDelete(); }),
		m_objects.end());

	// Deferred actions run last, once the object list is no longer being iterated. Move them out
	// first so an action that queues another (e.g. rebuilding the scene) defers to the next frame.
	if (!m_afterUpdate.empty())
	{
		auto actions = std::move(m_afterUpdate);
		m_afterUpdate.clear();
		for (auto &action : actions)
		{
			action();
		}
	}
}

void Scene::FixedUpdate()
{
	for (auto &object : m_objects)
	{
		object->FixedUpdate();
	}
}

void Scene::Render() const
{
	for (const auto &object : m_objects)
	{
		object->Render();
	}

    ServiceLocator::get_collision_grid().Render();
}
