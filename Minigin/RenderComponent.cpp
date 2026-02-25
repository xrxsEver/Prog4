#include "RenderComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"

dae::RenderComponent::RenderComponent(GameObject *pOwner)
    : Component(pOwner)
{
}

void dae::RenderComponent::Render() const
{
    RenderTextureAtOwnerPosition(m_texture);
}

void dae::RenderComponent::SetTexture(const std::string &filename)
{
    m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_texture = std::move(texture);
}
