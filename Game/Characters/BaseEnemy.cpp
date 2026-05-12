#include "BaseEnemy.h"

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
        (void)dir;
        return true; // Mock implementation
    }

    bool BaseEnemy::IsTileIce(const glm::vec2& dir) const
    {
        (void)dir;
        return false; // Mock implementation
    }

    void BaseEnemy::BreakBlockInDirection(const glm::vec2& dir)
    {
        (void)dir;
        // Mock implementation
    }

    glm::vec3 BaseEnemy::GetPlayerPosition() const
    {
        return glm::vec3(0.0f, 0.0f, 0.0f); // Mock implementation
    }
}