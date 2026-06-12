#pragma once
#include "Character.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace dae
{
    class BaseEnemy : public Character
    {
    public:
        BaseEnemy(std::string name, ResourceManager& resourceManager);
        // Note: Character/GameObject Update is not virtual in this engine design.
        // We will override it or just hide it, but wait, GameObject::Update is not virtual.
        // We will hide it or make a Component bridge.
        void UpdateFromComponent(float dt);

        // Who this enemy hunts (Pengo); without it the AI just chases the origin
        void SetTarget(const GameObject* pTarget) { m_pTarget = pTarget; }

    protected:
        virtual void PerformAction(float dt) = 0;

        const GameObject* m_pTarget{ nullptr };

        // Helper methods for subclasses
        void MoveTowards(const glm::vec2& dir);
        void BreakBlockFront();
        void PlaySound(int soundId);

        // New helpers for tile-based arcade AI
        bool IsTileWalkable(const glm::vec2& dir) const;
        bool IsTileIce(const glm::vec2& dir) const;
        void BreakBlockInDirection(const glm::vec2& dir);
        glm::vec3 GetPlayerPosition() const;
    };
}