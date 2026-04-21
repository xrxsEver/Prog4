#include "Character.h"

#include <algorithm>

#include "RenderComponent.h"

dae::Character::Character(std::string name, ResourceManager &resourceManager)
    : GameObject(std::move(name)), m_resourceManager(resourceManager)
{
}

void dae::Character::LoseLife()
{
    if (health <= 0)
    {
        return;
    }

    health = (std::max)(0, health - 1);
    NotifyObservers(GameEvent::PlayerDied);
}

void dae::Character::AddScore(const int points)
{
    if (points <= 0)
    {
        return;
    }

    score += points;
    NotifyObservers(GameEvent::PointsGained);
}

void dae::Character::InitializeSprite(const float sourceX, const float sourceY)
{
    auto *renderComponent = AddComponent<RenderComponent>(m_resourceManager);
    renderComponent->SetTexture("pengo.png");
    renderComponent->SetSourceRect(sourceX, sourceY, m_spriteSize, m_spriteSize);
    renderComponent->SetRenderSize(m_displaySize, m_displaySize);
}