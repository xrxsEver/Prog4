#include "IceBlockPool.h"
#include "Scene.h"
#include "ResourceManager.h"
#include <stdexcept>

namespace dae
{
    IceBlockPool::IceBlockPool(Scene&, ResourceManager& resourceManager, size_t poolSize)
    {
        m_pool.reserve(poolSize);
        for (size_t i = 0; i < poolSize; ++i)
        {
            auto block = std::make_unique<IceBlock>(resourceManager);
            block->Reset();
            
            m_pool.push_back(std::move(block));
        }
    }

    [[nodiscard]] IceBlock* IceBlockPool::Acquire()
    {
        for (const auto& block : m_pool)
        {
            if (!block->IsActive())
            {
                block->SetActive(true);
                return block.get();
            }
        }
        return nullptr;
    }

    void IceBlockPool::Release(IceBlock* pBlock)
    {
        if (pBlock)
        {
            pBlock->Reset();
        }
    }

    void IceBlockPool::Spawn(float x, float y)
    {
        IceBlock* block = Acquire();
        if (block)
        {
            block->SetPosition(x, y);
        }
    }

    void IceBlockPool::Update(float deltaTime)
    {
        for (auto& block : m_pool)
        {
            block->Update(deltaTime);
        }
    }

    void IceBlockPool::Render() const
    {
        for (const auto& block : m_pool)
        {
            block->Render();
        }
    }
}
