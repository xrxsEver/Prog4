#include "PushCommand.h"
#include "PengoCharacter.h"

namespace dae
{
    PushCommand::PushCommand(GameObject &actor)
        : GameActorCommand(actor)
    {
    }

    void PushCommand::Execute()
    {
        if (auto* pengo = dynamic_cast<PengoCharacter*>(&GetActor()))
        {
            pengo->Push();
        }
    }
}
