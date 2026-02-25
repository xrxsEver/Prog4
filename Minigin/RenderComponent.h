#pragma once
#include <memory>
#include <string>
#include <optional>
#include <SDL3/SDL_rect.h>
#include "Component.h"

namespace dae
{
    class Texture2D;

    class RenderComponent final : public Component
    {
    public:
        explicit RenderComponent(GameObject *pOwner);

        void Render() const override;

        void SetTexture(const std::string &filename);
        void SetTexture(std::shared_ptr<Texture2D> texture);
        void SetSourceRect(float x, float y, float w, float h);
        void SetRenderSize(float w, float h);

    private:
        std::shared_ptr<Texture2D> m_texture{};
        std::optional<SDL_FRect> m_srcRect{};
        std::optional<SDL_FRect> m_dstSize{};
    };
}
