#include "LoseLifeCommand.h"

#include "Character.h"

dae::LoseLifeCommand::LoseLifeCommand(GameObject &actor)
    : GameActorCommand(actor)
{
}

void dae::LoseLifeCommand::Execute()
{
    auto *character = dynamic_cast<Character *>(&GetActor());
    if (character == nullptr)
    {
        return;
    }

    character->LoseLife();
}
