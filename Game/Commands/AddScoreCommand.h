#pragma once

#include "GameActorCommand.h"

namespace dae
{
    class AddScoreCommand final : public GameActorCommand
    {
    public:
        AddScoreCommand(GameObject &actor, int points);
        void Execute() override;

    private:
        int m_points{};
    };
}
