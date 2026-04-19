#pragma once

#include "GameActorCommand.h"

namespace dae
{
    class LoseLifeCommand final : public GameActorCommand
    {
    public:
        explicit LoseLifeCommand(GameObject &actor);
        void Execute() override;
    };
}
