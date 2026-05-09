#include "MoveCommand.h"
#include "GameObject.h"
#include "GameTime.h"
#include "PengoCharacter.h"
#include "InputManager.h"

namespace dae
{
    MoveCommand::MoveCommand(GameObject &actor, const glm::vec2 direction, const float /*speed*/)
        : GameActorCommand(actor), m_direction(direction), m_speed(0.0f)
    {
    }

    void MoveCommand::Execute()
    {
        auto &actor = GetActor();

        // If the actor is a PengoCharacter, use its input priority system
        if (auto* pengo = dynamic_cast<PengoCharacter*>(&actor))
        {
            PengoDirection dir;
            if (m_direction.y > 0) dir = PengoDirection::Down;
            else if (m_direction.x < 0) dir = PengoDirection::Left;
            else if (m_direction.y < 0) dir = PengoDirection::Up;
            else if (m_direction.x > 0) dir = PengoDirection::Right;
            else return;

            pengo->AddMoveInput(dir);
        }
    }
}