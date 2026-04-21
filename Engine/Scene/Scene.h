#pragma once
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"

namespace dae
{
	class Scene final
	{
	public:
		void Add(std::unique_ptr<GameObject> object);
		void Remove(const GameObject &object);
		void RemoveAll();
		const std::vector<std::unique_ptr<GameObject>> &GetObjects() const { return m_objects; }

		void Update(float deltaTime);
		void FixedUpdate();
		void Render() const;

		explicit Scene() = default;

		~Scene() = default;
		Scene(const Scene &other) = delete;
		Scene(Scene &&other) = delete;
		Scene &operator=(const Scene &other) = delete;
		Scene &operator=(Scene &&other) = delete;

	private:
		friend class SceneManager;

		std::vector<std::unique_ptr<GameObject>> m_objects{};
	};

}
