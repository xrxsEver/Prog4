#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <glm/vec3.hpp>
#include "Transform.h"

namespace dae
{
	// Forward-declare Component to reduce header dependencies.
	class Component;

	class GameObject
	{
	public:
		void Update(float deltaTime);
		void FixedUpdate();
		void Render() const;

		void MarkForDelete();
		bool IsMarkedForDelete() const;

		void SetParent(GameObject *parent, bool keepWorldPosition = true);
		GameObject *GetParent() const { return m_pParent; }
		const std::vector<GameObject *> &GetChildren() const { return m_children; }

		void SetPosition(float x, float y);
		void SetLocalPosition(const glm::vec3 &pos);
		void SetLocalRotation(const glm::vec3 &rot);
		void SetLocalScale(const glm::vec3 &scale);
		const glm::vec3 &GetLocalPosition() const { return m_localPosition; }
		const glm::vec3 &GetLocalRotation() const { return m_localRotation; }
		const glm::vec3 &GetLocalScale() const { return m_localScale; }
		const glm::vec3 &GetWorldPosition() const;
		Transform &GetTransform();
		const Transform &GetTransform() const;
		void SetName(std::string name);
		const std::string &GetName() const { return m_name; }
		const std::vector<std::unique_ptr<Component>> &GetComponents() const { return m_components; }

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

		explicit GameObject(std::string name = "GameObject");
		virtual ~GameObject();
		GameObject(const GameObject &other) = delete;
		GameObject(GameObject &&other) = delete;
		GameObject &operator=(const GameObject &other) = delete;
		GameObject &operator=(GameObject &&other) = delete;

	private:
		void AddChild(GameObject *child);
		void RemoveChild(GameObject *child);
		void SetDirty();
		void UpdateWorldTransform() const;
		bool IsChild(const GameObject *object) const;

		// --- Transform Data ---
		/// Local position relative to the parent.
		glm::vec3 m_localPosition{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_localRotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_localScale{ 1.0f, 1.0f, 1.0f };
		/// Cached world transform.
		mutable Transform m_transform{};
		/// Flag to indicate that the world transform needs to be recalculated.
		mutable bool m_isDirty{ true };


		// --- Scene Graph Data ---
		/// Parent GameObject in the scene hierarchy.
		GameObject *m_pParent{ nullptr };
		/// Child GameObjects in the scene hierarchy.
		std::vector<GameObject *> m_children{};


		// --- Core Data ---
		/// Collection of components attached to this GameObject.
		std::vector<std::unique_ptr<Component>> m_components{};
		/// Name of the GameObject.
		std::string m_name{};
		/// Flag to mark the GameObject for deletion at the end of the frame.
		bool m_isMarkedForDelete{ false };
	};
}
