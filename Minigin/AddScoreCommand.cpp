#include "AddScoreCommand.h"

#include "Character.h"

dae::AddScoreCommand::AddScoreCommand(GameObject *pActor, const int points)
    : GameActorCommand(pActor), m_points(points)
{
}

void dae::AddScoreCommand::Execute()
{
    auto *character = dynamic_cast<Character *>(GetActor());
    if (character == nullptr)
    {
        return;
    }

    character->AddScore(m_points);
}
