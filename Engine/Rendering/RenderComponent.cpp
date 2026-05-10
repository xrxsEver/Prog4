#include "RenderComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include <imgui.h>

dae::RenderComponent::RenderComponent(GameObject *pOwner, ResourceManager &resourceManager)
    : Component(pOwner), m_pResourceManager(&resourceManager)
{
}

void dae::RenderComponent::Render() const
{
    if (!m_texture)
        return;

    const auto &pos = GetOwner()->GetTransform().position;

    if (m_srcRect.has_value())
    {
        const float dstW = m_dstSize ? m_dstSize->width : m_srcRect->width;
        const float dstH = m_dstSize ? m_dstSize->height : m_srcRect->height;
        Renderer::GetInstance().RenderTexture(*m_texture, *m_srcRect, pos.x, pos.y, dstW, dstH);
    }
    else if (m_dstSize.has_value())
    {
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, m_dstSize->width, m_dstSize->height);
    }
    else
    {
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
    }
}

void dae::RenderComponent::SetTexture(const std::string_view filename)
{
    if (m_pResourceManager == nullptr)
    {
        return;
    }

    m_texture = m_pResourceManager->LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
{
    m_texture = std::move(texture);
}

void dae::RenderComponent::SetSourceRect(float x, float y, float w, float h)
{
    m_srcRect = Rect{x, y, w, h};
}

void dae::RenderComponent::SetRenderSize(float w, float h)
{
    m_dstSize = Rect{0.f, 0.f, w, h};
}

std::unique_ptr<dae::Component> dae::RenderComponent::Clone(GameObject* pOwner) const
{
    auto clone = std::make_unique<RenderComponent>(pOwner, *m_pResourceManager);
    clone->m_texture = m_texture;
    clone->m_srcRect = m_srcRect;
    clone->m_dstSize = m_dstSize;
    return clone;
}

void dae::RenderComponent::DrawInspector() const
{
    ImGui::Text("Texture: %s", m_texture ? "loaded" : "none");

    if (m_srcRect)
    {
        ImGui::Text("Source: %.0f, %.0f, %.0f, %.0f", m_srcRect->x, m_srcRect->y, m_srcRect->width, m_srcRect->height);
    }
    else
    {
        ImGui::TextDisabled("Source: full texture");
    }

    if (m_dstSize)
    {
        ImGui::Text("Render size: %.0f x %.0f", m_dstSize->width, m_dstSize->height);
    }
}
