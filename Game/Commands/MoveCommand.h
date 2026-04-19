#pragma once
#include <glm/vec2.hpp>
#include "GameActorCommand.h"

namespace dae
{
    class MoveCommand final : public GameActorCommand
    {
    public:
        MoveCommand(GameObject &actor, glm::vec2 direction, float speed);
        void Execute() override;

    private:
        glm::vec2 m_direction{};
        float m_speed{};
    };
}
