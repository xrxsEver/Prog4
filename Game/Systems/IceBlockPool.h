#pragma once
#include <vector>
#include <memory>
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

        void Update(float deltaTime);
        void Render() const;

    private:
        std::vector<std::unique_ptr<IceBlock>> m_pool;
        // Tracking logic hidden inside the manager
    };
}
