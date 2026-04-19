#include "AddScoreCommand.h"

#include "Character.h"

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
}
