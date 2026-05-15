#pragma once

#include <vector>

namespace dae
{
    class SnoBeeCharacter;

    class SnoBeeManager
    {
    public:
        void AddSnoBee(SnoBeeCharacter* snoBee);
        void Update(float deltaTime);

    private:
        std::vector<SnoBeeCharacter*> m_snoBees;
    };
}
