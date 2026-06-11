#include "IceBlockPool.h"
#include "Scene.h"
#include "ResourceManager.h"
#include <stdexcept>
#include <cmath>

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

    std::vector<glm::vec3> IceBlockPool::GetActivePositions() const
    {
        std::vector<glm::vec3> positions;
        for (const auto& block : m_pool)
        {
            if (block->IsActive())
            {
                positions.push_back(block->GetLocalPosition());
            }
        }
        return positions;
    }

    void IceBlockPool::Restore(const std::vector<glm::vec3>& positions)
    {
        // Wipe the pool clean, then re-place a block at each remembered spot
        for (auto& block : m_pool)
        {
            block->Reset();
        }

        for (const auto& pos : positions)
        {
            if (IceBlock* block = Acquire())
            {
                block->SetPosition(pos.x, pos.y);
            }
        }
    }

    IceBlock* IceBlockPool::FindActiveAt(const glm::vec3& pos) const
    {
        for (const auto& block : m_pool)
        {
            if (!block->IsActive()) continue;
            const auto bp = block->GetLocalPosition();
            if (std::abs(bp.x - pos.x) < 1.0f && std::abs(bp.y - pos.y) < 1.0f)
            {
                return block.get();
            }
        }
        return nullptr;
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
