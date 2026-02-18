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
    if (!m_texture)
        return;

    const auto &pos = GetOwner()->GetTransform().GetPosition();
    Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void dae::RenderComponent::SetTexture(const std::string &filename)
{
    m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_texture = std::move(texture);
}
