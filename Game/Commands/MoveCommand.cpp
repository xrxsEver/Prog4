#include "MoveCommand.h"
#include "GameObject.h"
#include "GameTime.h"
#include "PengoCharacter.h"

namespace dae
{
    MoveCommand::MoveCommand(GameObject &actor, const glm::vec2 direction, const float speed)
        : GameActorCommand(actor), m_direction(direction), m_speed(speed)
    {
    }

    void MoveCommand::Execute()
    {
        const float dt = GameTime::GetInstance().GetDeltaTime();
        auto &actor = GetActor();

        // If the actor is a PengoCharacter, update its current direction
        if (auto* pengo = dynamic_cast<PengoCharacter*>(&actor))
        {
            // Convert glm::vec2 direction to PengoDirection enum
            if (m_direction.y > 0) pengo->SetDirection(PengoDirection::Down);
            else if (m_direction.x < 0) pengo->SetDirection(PengoDirection::Left);
            else if (m_direction.y < 0) pengo->SetDirection(PengoDirection::Up);
            else if (m_direction.x > 0) pengo->SetDirection(PengoDirection::Right);
        }

        const glm::vec3 currentPos = actor.GetLocalPosition();
        const glm::vec3 movement{m_direction.x * m_speed * dt, m_direction.y * m_speed * dt, 0.0f};
        actor.SetLocalPosition(currentPos + movement);
    }
}