#include "Character.h"

#include <algorithm>

#include "RenderComponent.h"

dae::Character::Character(std::string name)
    : GameObject(std::move(name))
{
}

int dae::Character::GetHealth() const
{
    return m_health;
}

int dae::Character::GetScore() const
{
    return m_score;
}

void dae::Character::LoseLife()
{
    if (m_health <= 0)
    {
        return;
    }

    m_health = (std::max)(0, m_health - 1);
    NotifyObservers(GameEvent::PlayerDied);
}

void dae::Character::AddScore(const int points)
{
    if (points <= 0)
    {
        return;
    }

    m_score += points;
    NotifyObservers(GameEvent::PointsGained);
}

void dae::Character::InitializeSprite(const float sourceX, const float sourceY)
{
    auto *renderComponent = AddComponent<RenderComponent>();
    renderComponent->SetTexture("pengo.png");
    renderComponent->SetSourceRect(sourceX, sourceY, m_spriteSize, m_spriteSize);
    renderComponent->SetRenderSize(m_displaySize, m_displaySize);
}