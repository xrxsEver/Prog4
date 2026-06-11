#pragma once
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include "IceBlock.h"

namespace dae
{
    class Scene;
    class ResourceManager;

    class IceBlockPool final
    {
    public:
        explicit IceBlockPool(Scene& scene, ResourceManager& resourceManager, size_t poolSize = 200);
        ~IceBlockPool() = default;

        IceBlockPool(const IceBlockPool&) = delete;
        IceBlockPool& operator=(const IceBlockPool&) = delete;
        IceBlockPool(IceBlockPool&&) = delete;
        IceBlockPool& operator=(IceBlockPool&&) = delete;

        [[nodiscard]] IceBlock* Acquire();
        void Release(IceBlock* pBlock);

        // Encapsulated interface (I.30)
        void Spawn(float x, float y);

        // Snapshot the live blocks (for remembering the maze) and put them back later
        [[nodiscard]] std::vector<glm::vec3> GetActivePositions() const;
        void Restore(const std::vector<glm::vec3>& positions);

        // Find the active block sitting on a given tile (used to re-mark eggs after a respawn)
        [[nodiscard]] IceBlock* FindActiveAt(const glm::vec3& pos) const;

        void Update(float deltaTime);
        void Render() const;

    private:
        std::vector<std::unique_ptr<IceBlock>> m_pool;
        // Tracking logic hidden inside the manager
    };
}
