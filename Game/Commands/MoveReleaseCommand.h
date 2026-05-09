#pragma once
#include <glm/vec2.hpp>
#include "GameActorCommand.h"
#include "PengoCharacter.h"

namespace dae
{
    class MoveReleaseCommand final : public GameActorCommand
    {
    public:
        MoveReleaseCommand(GameObject &actor, glm::vec2 direction)
            : GameActorCommand(actor), m_direction(direction) {}
        
        void Execute() override
        {
            if (auto* pengo = dynamic_cast<PengoCharacter*>(&GetActor()))
            {
                PengoDirection dir;
                if (m_direction.y > 0) dir = PengoDirection::Down;
                else if (m_direction.x < 0) dir = PengoDirection::Left;
                else if (m_direction.y < 0) dir = PengoDirection::Up;
                else if (m_direction.x > 0) dir = PengoDirection::Right;
                else return;

                pengo->RemoveMoveInput(dir);
            }
        }

    private:
        glm::vec2 m_direction{};
    };
}
