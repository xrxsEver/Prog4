#include "GameObject.h"
#include "Component.h"

#include <algorithm>

dae::GameObject::~GameObject()
{
	// Detach from parent
	if (m_pParent)
	{
		m_pParent->RemoveChild(this);
	}

	// Orphan all children
	for (auto *child : m_children)
	{
		child->m_pParent = nullptr;
		child->SetPositionDirty();
	}
	m_children.clear();
}

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
	m_localPosition = {x, y, 0.0f};
	SetPositionDirty();
}

void dae::GameObject::SetLocalPosition(const glm::vec3 &pos)
{
	m_localPosition = pos;
	SetPositionDirty();
}

const glm::vec3 &dae::GameObject::GetWorldPosition()
{
	UpdateWorldPosition();
	return m_transform.GetPosition();
}

void dae::GameObject::MarkForDelete()
{
	m_isMarkedForDelete = true;
}

bool dae::GameObject::IsMarkedForDelete() const
{
	return m_isMarkedForDelete;
}

// --- Scenegraph hierarchy (SetParent-centric) ---
void dae::GameObject::SetParent(GameObject *parent, bool keepWorldPosition)
{
	// 1. Validate: cannot parent to self or to one of own children
	if (parent == this || IsChild(parent))
	{
		return;
	}

	// Already the current parent – nothing to do
	if (m_pParent == parent)
	{
		return;
	}

	// 2. Update local position so world position stays the same
	if (keepWorldPosition)
	{
		if (parent)
		{
			SetLocalPosition(GetWorldPosition() - parent->GetWorldPosition());
		}
		else
		{
			SetLocalPosition(GetWorldPosition());
		}
	}

	// 3. Remove itself from previous parent
	if (m_pParent)
	{
		m_pParent->RemoveChild(this);
	}

	// 4. Set the given parent on itself
	m_pParent = parent;

	// 5. Add itself as a child to the given parent
	if (m_pParent)
	{
		m_pParent->AddChild(this);
	}
	SetPositionDirty();
}

// --- helpers ---

void dae::GameObject::AddChild(GameObject *child)
{
	m_children.emplace_back(child);
}

void dae::GameObject::RemoveChild(GameObject *child)
{
	m_children.erase(
		std::remove(m_children.begin(), m_children.end(), child),
		m_children.end());
}

dae::Transform &dae::GameObject::GetTransform()
{
	UpdateWorldPosition();
	return m_transform;
}

const dae::Transform &dae::GameObject::GetTransform() const
{
	UpdateWorldPosition();
	return m_transform;
}

void dae::GameObject::SetPositionDirty()
{
	m_isPositionDirty = true;

	for (auto *child : m_children)
	{
		child->SetPositionDirty();
	}
}

void dae::GameObject::UpdateWorldPosition() const
{
	if (!m_isPositionDirty)
	{
		return;
	}

	glm::vec3 worldPosition = m_localPosition;
	if (m_pParent != nullptr)
	{
		worldPosition += m_pParent->GetTransform().GetPosition();
	}
	m_transform.SetPosition(worldPosition);
	m_isPositionDirty = false;
}

bool dae::GameObject::IsChild(const GameObject *object) const
{
	if (object == nullptr)
	{
		return false;
	}

	for (const auto *child : m_children)
	{
		if (child == object || child->IsChild(object))
		{
			return true;
		}
	}
	return false;
}
