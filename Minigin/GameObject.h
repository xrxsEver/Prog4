#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <glm/vec3.hpp>
#include "Transform.h"

namespace dae
{
	class Component;

	class GameObject final
	{
	public:
		void Update();
		void FixedUpdate();
		void Render() const;

		void MarkForDelete();
		bool IsMarkedForDelete() const;

		void SetParent(GameObject *parent, bool keepWorldPosition = true);
		GameObject *GetParent() const { return m_pParent; }
		const std::vector<GameObject *> &GetChildren() const { return m_children; }

		void SetPosition(float x, float y);
		void SetLocalPosition(const glm::vec3 &pos);
		const glm::vec3 &GetLocalPosition() const { return m_localPosition; }
		const glm::vec3 &GetWorldPosition();
		Transform &GetTransform();
		const Transform &GetTransform() const;

		// component system
		template <typename T, typename... Args>
		T *AddComponent(Args &&...args)
		{
			auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
			T *rawPtr = component.get();
			m_components.emplace_back(std::move(component));
			return rawPtr;
		}

		template <typename T>
		T *GetComponent() const
		{
			for (const auto &comp : m_components)
			{
				T *casted = dynamic_cast<T *>(comp.get());
				if (casted)
					return casted;
			}
			return nullptr;
		}

		template <typename T>
		bool HasComponent() const
		{
			return GetComponent<T>() != nullptr;
		}

		template <typename T>
		void RemoveComponent()
		{
			m_components.erase(
				std::remove_if(m_components.begin(), m_components.end(),
							   [](const auto &comp)
							   { return dynamic_cast<T *>(comp.get()) != nullptr; }),
				m_components.end());
		}

		GameObject() = default;
		~GameObject();
		GameObject(const GameObject &other) = delete;
		GameObject(GameObject &&other) = delete;
		GameObject &operator=(const GameObject &other) = delete;
		GameObject &operator=(GameObject &&other) = delete;

	private:
		void AddChild(GameObject *child);
		void RemoveChild(GameObject *child);
		void SetPositionDirty();
		void UpdateWorldPosition() const;
		bool IsChild(const GameObject *object) const;

		mutable Transform m_transform{};
		glm::vec3 m_localPosition{};
		mutable bool m_isPositionDirty{true};
		GameObject *m_pParent{};
		std::vector<GameObject *> m_children{};
		std::vector<std::unique_ptr<Component>> m_components{};
		bool m_isMarkedForDelete{};
	};
}
