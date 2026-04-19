#pragma once
#include <memory>
#include <string_view>
#include <optional>
#include <SDL3/SDL_rect.h>
#include "Component.h"

namespace dae
{
    class Texture2D;
    class ResourceManager;

    class RenderComponent final : public Component
    {
    public:
        RenderComponent(GameObject *pOwner, ResourceManager &resourceManager);

        void Render() const override;
        const char *GetDebugName() const override { return "Render"; }
        void DrawInspector() const override;

        void SetTexture(std::string_view filename);
        void SetTexture(std::shared_ptr<Texture2D> texture);
        void SetSourceRect(float x, float y, float w, float h);
        void SetRenderSize(float w, float h);

    private:
        ResourceManager *m_pResourceManager{};
        std::shared_ptr<Texture2D> m_texture{};
        std::optional<SDL_FRect> m_srcRect{};
        std::optional<SDL_FRect> m_dstSize{};
    };
}
