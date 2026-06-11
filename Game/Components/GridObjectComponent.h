#pragma once
#include <memory>
#include "Component.h"
#include "CollisionGrid.h"
#include "GameObject.h"
#include "ServiceLocator.h"

namespace dae
{
    class GridObjectComponent final : public Component
    {
    public:
        explicit GridObjectComponent(GameObject* pOwner)
            : Component(pOwner)
            , m_lastPos(pOwner->GetWorldPosition())
            , m_isEnabled(true)
        {
            ServiceLocator::get_collision_grid().RegisterObject(GetOwner());
        }

        virtual ~GridObjectComponent() override
        {
            if (m_isEnabled)
            {
                ServiceLocator::get_collision_grid().UnregisterObject(GetOwner(), m_lastPos);
            }
        }

        void Enable()
        {
            if (m_isEnabled) return;

            m_lastPos = GetOwner()->GetWorldPosition();
            ServiceLocator::get_collision_grid().RegisterObject(GetOwner());
            m_isEnabled = true;
        }

        void Disable()
        {
            if (!m_isEnabled) return;

            ServiceLocator::get_collision_grid().UnregisterObject(GetOwner(), m_lastPos);
            m_isEnabled = false;
        }

        bool IsEnabled() const { return m_isEnabled; }

        // Push our current position into the grid immediately, rather than waiting for the next
        // Update. Used when an ice block shoves a Sno-Bee and later queries must see the new cell.
        void SyncToCurrentPosition()
        {
            if (!m_isEnabled) return;

            const glm::vec3& currentPos = GetOwner()->GetWorldPosition();
            if (currentPos != m_lastPos)
            {
                ServiceLocator::get_collision_grid().UpdateObject(GetOwner(), m_lastPos, currentPos);
                m_lastPos = currentPos;
            }
        }

        virtual void Update(float /*deltaTime*/) override
        {
            if (!m_isEnabled) return;

            const glm::vec3& currentPos = GetOwner()->GetWorldPosition();
            if (currentPos != m_lastPos)
            {
                ServiceLocator::get_collision_grid().UpdateObject(GetOwner(), m_lastPos, currentPos);
                m_lastPos = currentPos;
            }
        }

        virtual std::unique_ptr<Component> Clone(GameObject* pOwner) const override
        {
            return std::make_unique<GridObjectComponent>(pOwner);
        }

    private:
        glm::vec3 m_lastPos;
        bool m_isEnabled;
    };
}
