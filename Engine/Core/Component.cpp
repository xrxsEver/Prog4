#include "Component.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Texture2D.h"

void dae::Component::RenderTextureAtOwnerPosition(const std::shared_ptr<Texture2D> &texture) const
{
    if (!texture)
        return;

    const auto &pos = GetOwner()->GetTransform().GetPosition();
    Renderer::GetInstance().RenderTexture(*texture, pos.x, pos.y);
}
