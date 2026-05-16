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
            if (dynamic_cast<IceBlock*>(obj)) return true;
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
                iceBlock->Reset();
                // Grid update will be handled by Reset calling SetPosition or by IceBlock's GridObjectComponent
                return;
            }
        }
    }

    glm::vec3 BaseEnemy::GetPlayerPosition() const
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
}