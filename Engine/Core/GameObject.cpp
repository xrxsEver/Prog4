#include "GameObject.h"
#include "Component.h"

#include <algorithm>

dae::GameObject::GameObject(std::string name)
	: m_localPosition(0.0f, 0.0f, 0.0f)
	, m_localRotation(0.0f, 0.0f, 0.0f)
	, m_localScale(1.0f, 1.0f, 1.0f)
	, m_transform()
	, m_isDirty(true)
	, m_pParent(nullptr)
	, m_children()
	, m_components()
	, m_name(std::move(name))
	, m_isMarkedForDelete(false)
{
}

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
		child->SetDirty();
	}
	m_children.clear();
}
// this can be changed to a simpler destrucor ,!gameobject()= defaulte

void dae::GameObject::Update(float deltaTime)
{
	for (auto &comp : m_components)
	{
		comp->Update(deltaTime);
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
	SetDirty();
}

void dae::GameObject::SetLocalPosition(const glm::vec3 &pos)
{
	m_localPosition = pos;
	SetDirty();
}

void dae::GameObject::SetLocalRotation(const glm::vec3 &rot)
{
	m_localRotation = rot;
	SetDirty();
}

void dae::GameObject::SetLocalScale(const glm::vec3 &scale)
{
	m_localScale = scale;
	SetDirty();
}

const glm::vec3 &dae::GameObject::GetWorldPosition() const
{
	UpdateWorldTransform();
	return m_transform.position;
}

void dae::GameObject::SetName(std::string name)
{
	m_name = std::move(name);
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
	SetDirty();
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
	UpdateWorldTransform();
	return m_transform;
}

const dae::Transform &dae::GameObject::GetTransform() const
{
	UpdateWorldTransform();
	return m_transform;
}

void dae::GameObject::SetDirty()
{
	m_isDirty = true;

	for (auto *child : m_children)
	{
		child->SetDirty();
	}
}

void dae::GameObject::UpdateWorldTransform() const
{
	if (!m_isDirty)
	{
		return;
	}

	if (m_pParent != nullptr)
	{
		const auto& parentTransform = m_pParent->GetTransform();
		m_transform.position = parentTransform.position + m_localPosition;
		m_transform.rotation = parentTransform.rotation + m_localRotation;
		m_transform.scale = parentTransform.scale * m_localScale;
	}
	else
	{
		m_transform.position = m_localPosition;
		m_transform.rotation = m_localRotation;
		m_transform.scale = m_localScale;
	}

	m_isDirty = false;
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
