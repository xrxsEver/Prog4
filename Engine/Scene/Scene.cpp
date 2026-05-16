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
	m_objects.clear();
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

	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[](const auto &object)
			{ return object->IsMarkedForDelete(); }),
		m_objects.end());
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
