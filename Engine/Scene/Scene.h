#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"
#include "SnoBeeManager.h"

namespace dae
{
    class SnoBeeCharacter;

	class Scene final
	{
	public:
		void Add(std::unique_ptr<GameObject> object);
        void AddSnoBee(SnoBeeCharacter* snoBee);
		void Remove(const GameObject &object);
		void RemoveAll();
		const std::vector<std::unique_ptr<GameObject>> &GetObjects() const { return m_objects; }

		// Queue a callback to run once at the very end of the next Update, after all objects have
		// updated and the delete-sweep has run. This is the safe place to tear down/rebuild a scene
		// from inside one of its own objects (e.g. a level transition), where clearing the object
		// list mid-iteration would be a use-after-free.
		void RunAfterUpdate(std::function<void()> action);

		void Update(float deltaTime);
		void FixedUpdate();
		void Render() const;

		explicit Scene();

		~Scene() = default;
		Scene(const Scene &other) = delete;
		Scene(Scene &&other) = delete;
		Scene &operator=(const Scene &other) = delete;
		Scene &operator=(Scene &&other) = delete;

	private:
		friend class SceneManager;

		std::vector<std::unique_ptr<GameObject>> m_objects{};
		std::vector<std::unique_ptr<GameObject>> m_objectsToAdd{};
		std::vector<std::function<void()>> m_afterUpdate{};
        std::unique_ptr<SnoBeeManager> m_snoBeeManager;
	};

}
