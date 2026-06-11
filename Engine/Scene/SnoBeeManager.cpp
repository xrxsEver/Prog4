#include "SnoBeeManager.h"
#include "../../Game/Characters/SnoBeeCharacter.h"

#include <algorithm>

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

    void SnoBeeManager::RemoveMarked()
    {
        // Drop our raw pointers to any Sno-Bee that is about to be freed by the scene
        m_snoBees.erase(
            std::remove_if(m_snoBees.begin(), m_snoBees.end(),
                [](SnoBeeCharacter* snoBee) { return snoBee->IsMarkedForDelete(); }),
            m_snoBees.end());
    }
}
