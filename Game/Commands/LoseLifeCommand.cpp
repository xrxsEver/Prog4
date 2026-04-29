#include "LoseLifeCommand.h"
#include "Character.h"
#include "ServiceLocator.h"

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
    if (character->health > 0)
    {
        ServiceLocator::get_sound_system().play(1, 0.5f);
    }
    else
    {
        ServiceLocator::get_sound_system().play_music("Sounds/Miss.mp3", 0.6f, false);
    }
}
