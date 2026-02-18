#include "GameObject.h"
#include "Component.h"

dae::GameObject::~GameObject() = default;

void dae::GameObject::Update()
{
	for (auto &comp : m_components)
	{
		comp->Update();
	}
}

void dae::GameObject::FixedUpdate()
{
	for (auto &comp : m_components)
	{
		comp->FixedUpdate();
	}
}

void dae::GameObject::Render() const
{
	for (const auto &comp : m_components)
	{
		comp->Render();
	}
}

void dae::GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}
