#pragma once
#include "Command.h"

namespace dae
{
    class GameObject;

    class GameActorCommand : public Command
    {
    public:
        explicit GameActorCommand(GameObject* pActor)
            : m_pActor(pActor)
        {
        }

    protected:
        GameObject* GetActor() const { return m_pActor; }

    private:
        GameObject* m_pActor{};
    };
}
