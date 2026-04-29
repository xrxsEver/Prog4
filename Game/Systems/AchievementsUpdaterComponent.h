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
            
        virtual void Update(float deltaTime) override
        {
            (void)deltaTime;
            if (m_achievements) m_achievements->Update();
        }
        
        const char* GetDebugName() const override { return "Achievements Updater"; }

    private:
        Achievements* m_achievements;
    };
}
