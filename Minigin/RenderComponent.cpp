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

    if (m_srcRect.has_value())
    {
        const float dstW = m_dstSize ? m_dstSize->w : m_srcRect->w;
        const float dstH = m_dstSize ? m_dstSize->h : m_srcRect->h;
        Renderer::GetInstance().RenderTexture(*m_texture, *m_srcRect, pos.x, pos.y, dstW, dstH);
    }
    else if (m_dstSize.has_value())
    {
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, m_dstSize->w, m_dstSize->h);
    }
    else
    {
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
    }
}

void dae::RenderComponent::SetTexture(const std::string &filename)
{
    m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_texture = std::move(texture);
}

void dae::RenderComponent::SetSourceRect(float x, float y, float w, float h)
{
    m_srcRect = SDL_FRect{x, y, w, h};
}

void dae::RenderComponent::SetRenderSize(float w, float h)
{
    m_dstSize = SDL_FRect{0.f, 0.f, w, h};
}
