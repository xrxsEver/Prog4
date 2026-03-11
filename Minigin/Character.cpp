#include "Character.h"
#include "RenderComponent.h"

dae::Character::Character(std::string name)
    : GameObject(std::move(name))
{
}

void dae::Character::InitializeSprite(const float sourceX, const float sourceY)
{
    auto *renderComponent = AddComponent<RenderComponent>();
    renderComponent->SetTexture("pengo.png");
    renderComponent->SetSourceRect(sourceX, sourceY, m_spriteSize, m_spriteSize);
    renderComponent->SetRenderSize(m_displaySize, m_displaySize);
}