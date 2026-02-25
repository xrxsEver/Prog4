#pragma once
#include <vector>
#include <memory>
#include <algorithm>
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

		void SetPosition(float x, float y);
		Transform &GetTransform() { return m_transform; }
		const Transform &GetTransform() const { return m_transform; }

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
		Transform m_transform{};
		std::vector<std::unique_ptr<Component>> m_components{};
		bool m_isMarkedForDelete{};
	};
}
