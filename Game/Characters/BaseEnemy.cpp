#include "BaseEnemy.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "IceBlock.h"

namespace dae
{
    BaseEnemy::BaseEnemy(std::string name, ResourceManager& resourceManager)
        : Character(std::move(name), resourceManager)
    {
    }

    void BaseEnemy::UpdateFromComponent(float dt)
    {
        PerformAction(dt);
    }

    void BaseEnemy::MoveTowards(const glm::vec2& dir)
    {
        (void)dir;
    }

    void BaseEnemy::BreakBlockFront()
    {
    }

    void BaseEnemy::PlaySound(int soundId)
    {
        (void)soundId;
    }

    bool BaseEnemy::IsTileWalkable(const glm::vec2& dir) const
    {
        const auto currentPos = GetWorldPosition();
        const auto targetPos = currentPos + glm::vec3(dir.x * 32.0f, dir.y * 32.0f, 0.0f);
        
        const auto& grid = ServiceLocator::get_collision_grid();
        const auto [row, col] = grid.WorldToGrid(targetPos);

        if (!grid.IsWithinBounds(row, col)) return false;

        const auto objects = grid.GetObjectsAt(row, col);
        for (auto* obj : objects)
        {
            if (dynamic_cast<IceBlock*>(obj)) return false;
        }

        return true;
    }

    bool BaseEnemy::IsTileIce(const glm::vec2& dir) const
    {
        const auto currentPos = GetWorldPosition();
        const auto targetPos = currentPos + glm::vec3(dir.x * 32.0f, dir.y * 32.0f, 0.0f);

        const auto& grid = ServiceLocator::get_collision_grid();
        const auto [row, col] = grid.WorldToGrid(targetPos);

        if (!grid.IsWithinBounds(row, col)) return false;

        const auto objects = grid.GetObjectsAt(row, col);
        for (auto* obj : objects)
        {
            if (auto* iceBlock = dynamic_cast<IceBlock*>(obj))
            {
                // Eggs and diamonds are off-limits to Sno-Bee crushing
                if (iceBlock->HasEgg() || iceBlock->IsDiamond()) continue;
                return true;
            }
        }

        return false;
    }

    void BaseEnemy::BreakBlockInDirection(const glm::vec2& dir)
    {
        const auto currentPos = GetWorldPosition();
        const auto targetPos = currentPos + glm::vec3(dir.x * 32.0f, dir.y * 32.0f, 0.0f);

        const auto& grid = ServiceLocator::get_collision_grid();
        const auto [row, col] = grid.WorldToGrid(targetPos);

        if (!grid.IsWithinBounds(row, col)) return;

        const auto objects = grid.GetObjectsAt(row, col);
        for (auto* obj : objects)
        {
            if (auto* iceBlock = dynamic_cast<IceBlock*>(obj))
            {
                if (iceBlock->HasEgg() || iceBlock->IsDiamond()) continue; // never shatter eggs or diamonds
                iceBlock->Crush(); // play the shatter animation, then it removes itself
                return;
            }
        }
    }

    glm::vec3 BaseEnemy::GetPlayerPosition() const
    {
        // Chase the real target if we have one; otherwise fall back to the origin
        return m_pTarget ? m_pTarget->GetWorldPosition() : glm::vec3(0.0f, 0.0f, 0.0f);
    }
}