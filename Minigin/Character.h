#pragma once

#include <string>
#include "GameObject.h"
#include "Subject.h"

namespace dae
{
    class Character : public GameObject, public Subject
    {
    public:
        explicit Character(std::string name);
        int GetHealth() const;
        int GetScore() const;
        void LoseLife();
        void AddScore(int points);

    protected:
        void InitializeSprite(float sourceX, float sourceY);

    private:
        static constexpr float m_spriteSize{16.0f};
        static constexpr float m_displaySize{32.0f};
        static constexpr int m_startHealth{5};

    protected:
        int m_health{m_startHealth};
        int m_score{0};
    };
}