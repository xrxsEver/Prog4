#include "SnoBeeCounterComponent.h"
#include "MazeDrawingComponent.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "Renderer.h"

namespace dae
{
    SnoBeeCounterComponent::SnoBeeCounterComponent(GameObject* pOwner, ResourceManager& resourceManager, MazeDrawingComponent* pMaze)
        : Component(pOwner), m_pResourceManager(&resourceManager), m_pMaze(pMaze)
    {
        m_miscTexture = resourceManager.LoadTexture("misc.png");
    }

    void SnoBeeCounterComponent::Update(float deltaTime)
    {
        // Toggle between the two circle frames so the reserve looks alive
        m_flashTimer += deltaTime;
        if (m_flashTimer >= FLASH_INTERVAL)
        {
            m_flashTimer -= FLASH_INTERVAL;
            m_flashOn = !m_flashOn;
        }
    }

    void SnoBeeCounterComponent::Render() const
    {
        if (!m_miscTexture || m_pMaze == nullptr) return;

        const float srcX = ICON_SOURCE_X + (m_flashOn ? ICON_FRAME_STRIDE : 0.0f);
        const Rect src{srcX, ICON_SOURCE_Y, ICON_SOURCE_SIZE, ICON_SOURCE_SIZE};

        const auto& pos = GetOwner()->GetWorldPosition();

        // One circle per Sno-Bee still in reserve, stacked downwards
        for (int i = 0; i < m_pMaze->GetSnoBeeReserve(); ++i)
        {
            const float y = pos.y + static_cast<float>(i) * (ICON_DRAW_SIZE + ICON_SPACING);
            Renderer::GetInstance().RenderTexture(*m_miscTexture, src, pos.x, y, ICON_DRAW_SIZE, ICON_DRAW_SIZE);
        }
    }

    std::unique_ptr<Component> SnoBeeCounterComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<SnoBeeCounterComponent>(pOwner, *m_pResourceManager, m_pMaze);
    }
}
