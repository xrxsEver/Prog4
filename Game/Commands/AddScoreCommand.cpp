#include "AddScoreCommand.h"
#include "Character.h"
#include "ServiceLocator.h"

dae::AddScoreCommand::AddScoreCommand(GameObject &actor, const int points)
    : GameActorCommand(actor), m_points(points)
{
}

void dae::AddScoreCommand::Execute()
{
    auto *character = dynamic_cast<Character *>(&GetActor());
    if (character == nullptr)
    {
        return;
    }

    character->AddScore(m_points);
    ServiceLocator::get_sound_system().play(0, 0.4f);
}
