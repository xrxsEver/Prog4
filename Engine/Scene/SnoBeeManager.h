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
        // Forget pointers to Sno-Bees flagged for deletion (call before the scene frees them)
        void RemoveMarked();

    private:
        std::vector<SnoBeeCharacter*> m_snoBees;
    };
}
