#pragma once
#include "Component.h"
#include "Achievements.h"

namespace dae
{
    class AchievementsUpdaterComponent final : public Component
    {
    public:
        explicit AchievementsUpdaterComponent(GameObject* owner, Achievements* achievements)
            : Component(owner), m_achievements(achievements) {}

        void Update(float /*deltaTime*/) override
        {
            if (m_achievements)
            {
                m_achievements->Update();
            }
        }

        std::unique_ptr<Component> Clone(GameObject* pOwner) const override
        {
            return std::make_unique<AchievementsUpdaterComponent>(pOwner, m_achievements);
        }

    private:
        Achievements* m_achievements;
    };
}
