#pragma once
#include "GameActorCommand.h"

namespace dae
{
    class PushCommand final : public GameActorCommand
    {
    public:
        explicit PushCommand(GameObject &actor);
        void Execute() override;
    };
}
