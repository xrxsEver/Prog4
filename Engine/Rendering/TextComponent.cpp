#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>
#include "TextComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Font.h"
#include "Texture2D.h"
#include <imgui.h>
#include <memory>

namespace
{
    struct SDLSurfaceDeleter final
    {
        void operator()(SDL_Surface *surface) const noexcept
        {
            if (surface != nullptr)
            {
                SDL_DestroySurface(surface);
            }
        }
    };

    struct SDLTextureDeleter final
    {
        void operator()(SDL_Texture *texture) const noexcept
        {
            if (texture != nullptr)
            {
                SDL_DestroyTexture(texture);
            }
        }
    };

    using SurfacePtr = std::unique_ptr<SDL_Surface, SDLSurfaceDeleter>;
    using TexturePtr = std::unique_ptr<SDL_Texture, SDLTextureDeleter>;
}

dae::TextComponent::TextComponent(GameObject *pOwner, const std::string &text, std::shared_ptr<Font> font, const SDL_Color &color)
    : Component(pOwner), m_text(text), m_color(color), m_font(std::move(font))
{
}

void dae::TextComponent::Update()
{
    if (m_needsUpdate)
    {
        SurfacePtr surf{TTF_RenderText_Blended(m_font->GetFont(), m_text.c_str(), m_text.length(), m_color)};
        if (surf == nullptr)
        {
            throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());
        }

        TexturePtr texture{SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf.get())};
        if (texture == nullptr)
        {
            throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());
        }

        m_textTexture = std::make_shared<Texture2D>(texture.release());
        m_needsUpdate = false;
    }
}

void dae::TextComponent::Render() const
{
    RenderTextureAtOwnerPosition(m_textTexture);
}

void dae::TextComponent::SetText(const std::string &text)
{
    m_text = text;
    m_needsUpdate = true;
}

void dae::TextComponent::SetColor(const SDL_Color &color)
{
    m_color = color;
    m_needsUpdate = true;
}

void dae::TextComponent::DrawInspector() const
{
    ImGui::TextWrapped("Text: %s", m_text.c_str());
    ImGui::Text("Color: %u, %u, %u, %u", m_color.r, m_color.g, m_color.b, m_color.a);
    ImGui::Text("Dirty: %s", m_needsUpdate ? "yes" : "no");
}
