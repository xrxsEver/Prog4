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
    m_pRenderComponent = AddComponent<RenderComponent>(m_resourceManager);
    m_pRenderComponent->SetTexture("pengo.png");
    m_pRenderComponent->SetSourceRect(sourceX, sourceY, m_spriteSize, m_spriteSize);
    m_pRenderComponent->SetRenderSize(m_displaySize, m_displaySize);
}

void dae::Character::SetSpriteTexture(std::string_view filename)
{
    if (m_pRenderComponent) m_pRenderComponent->SetTexture(filename);
}

void dae::Character::SetSpriteSourceRect(float x, float y, float w, float h)
{
    if (m_pRenderComponent) m_pRenderComponent->SetSourceRect(x, y, w, h);
}

void dae::Character::SetSpriteRenderSize(float w, float h)
{
    if (m_pRenderComponent) m_pRenderComponent->SetRenderSize(w, h);
}
