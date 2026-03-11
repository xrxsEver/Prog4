#include "MoveCommand.h"
#include "GameObject.h"
#include "GameTime.h"

dae::MoveCommand::MoveCommand(GameObject* pActor, const glm::vec2 direction, const float speed)
    : GameActorCommand(pActor)
    , m_direction(direction)
    , m_speed(speed)
{
}

void dae::MoveCommand::Execute()
{
    auto* pActor = GetActor();
    if (pActor == nullptr)
    {
        return;
    }

    const float dt = GameTime::GetInstance().GetDeltaTime();
    const glm::vec3 currentPos = pActor->GetLocalPosition();
    const glm::vec3 movement{m_direction.x * m_speed * dt, m_direction.y * m_speed * dt, 0.0f};
    pActor->SetLocalPosition(currentPos + movement);
}
