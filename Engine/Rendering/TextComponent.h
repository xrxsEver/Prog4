#pragma once
#include <string>
#include <memory>
#include "Component.h"
#include "Renderer.h"

struct SDL_Color;

namespace dae
{
    class Font;
    class Texture2D;

    class TextComponent final : public Component
    {
    public:
        struct Color {
            unsigned char r, g, b, a;
        };

        TextComponent(GameObject *pOwner, const std::string &text, std::shared_ptr<Font> font, const Color &color = {255, 255, 255, 255});

        void Update(float deltaTime) override;
        void Render() const override;
        const char *GetDebugName() const override { return "Text"; }
        void DrawInspector() const override;
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

        void SetText(const std::string &text);
        void SetColor(const Color &color);

    private:
        bool m_needsUpdate{true};
        std::string m_text;
        Color m_color{255, 255, 255, 255};
        std::shared_ptr<Font> m_font;
        std::shared_ptr<Texture2D> m_textTexture;
    };
}
