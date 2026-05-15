#include "SnoBeeManager.h"
#include "../../Game/Characters/SnoBeeCharacter.h"

namespace dae
{
    void SnoBeeManager::AddSnoBee(SnoBeeCharacter* snoBee)
    {
        m_snoBees.push_back(snoBee);
    }

    void SnoBeeManager::Update(float deltaTime)
    {
        for (auto& snoBee : m_snoBees)
        {
            snoBee->Update(deltaTime);
        }
    }
}
