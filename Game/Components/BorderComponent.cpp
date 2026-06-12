#include "BorderComponent.h"
#include "GameObject.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "SnoBeeCharacter.h"

namespace dae
{
    // border.png frames: the left/right rattle is a vertical 8px strip drawn in two
    // frames (4px gap between them), the top/bottom one sits near the bottom of the file.
    static constexpr Rect VERT_FRAME[2] = { { 0.0f, 0.0f, 8.0f, 256.0f }, { 12.0f, 0.0f, 8.0f, 256.0f } };
    static constexpr Rect HORZ_FRAME[2] = { { 0.0f, 260.0f, 224.0f, 8.0f }, { 0.0f, 272.0f, 224.0f, 8.0f } };

    BorderComponent::BorderComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager)
        : Component(owner)
        , m_scene(scene)
        , m_resourceManager(resourceManager)
    {
        m_borderTexture = resourceManager.LoadTexture("border.png");
    }

    void BorderComponent::Update(float deltaTime)
    {
        if (m_activeEdge == Edge::None) return;

        m_timer -= deltaTime;
        if (m_timer <= 0.0f)
        {
            m_activeEdge = Edge::None;
            return;
        }

        m_flashTimer += deltaTime;
        if (m_flashTimer >= FLASH_INTERVAL)
        {
            m_flashTimer -= FLASH_INTERVAL;
            m_secondFrame = !m_secondFrame;
        }
    }

    void BorderComponent::Render() const
    {
        if (m_activeEdge == Edge::None || !m_borderTexture) return;

        const auto& grid = ServiceLocator::get_collision_grid();
        const glm::vec3 topLeft = grid.GridToWorld(0, 0); // inner top-left corner of the play area
        const float left = topLeft.x;
        const float top = topLeft.y;
        const float innerW = static_cast<float>(CollisionGrid::NUM_COLS * CollisionGrid::CELL_SIZE);
        const float innerH = static_cast<float>(CollisionGrid::NUM_ROWS * CollisionGrid::CELL_SIZE);
        const float right = left + innerW;
        const float bottom = top + innerH;

        // Cover the full wall thickness and run past the corners so the whole edge is lit
        const float outerW = innerW + 2.0f * THICKNESS;
        const float outerH = innerH + 2.0f * THICKNESS;

        auto& renderer = Renderer::GetInstance();
        const int frame = m_secondFrame ? 1 : 0;

        switch (m_activeEdge)
        {
        case Edge::Left:
            renderer.RenderTexture(*m_borderTexture, VERT_FRAME[frame], left - THICKNESS, top - THICKNESS, THICKNESS, outerH);
            break;
        case Edge::Right:
            renderer.RenderTexture(*m_borderTexture, VERT_FRAME[frame], right, top - THICKNESS, THICKNESS, outerH);
            break;
        case Edge::Top:
            renderer.RenderTexture(*m_borderTexture, HORZ_FRAME[frame], left - THICKNESS, top - THICKNESS, outerW, THICKNESS);
            break;
        case Edge::Bottom:
            renderer.RenderTexture(*m_borderTexture, HORZ_FRAME[frame], left - THICKNESS, bottom, outerW, THICKNESS);
            break;
        default: break;
        }
    }

    void BorderComponent::Push(Edge edge)
    {
        if (edge == Edge::None) return;
        m_activeEdge = edge;
        m_timer = VIBRATE_DURATION;
        m_flashTimer = 0.0f;
        m_secondFrame = false;
        StunSnoBeesAlong(edge);
    }

    void BorderComponent::StunSnoBeesAlong(Edge edge) const
    {
        const auto& grid = ServiceLocator::get_collision_grid();
        for (const auto& obj : m_scene.GetObjects())
        {
            auto* snoBee = dynamic_cast<SnoBeeCharacter*>(obj.get());
            if (!snoBee || snoBee->IsMarkedForDelete()) continue;

            const auto [row, col] = grid.WorldToGrid(snoBee->GetWorldPosition());
            bool onBorder = false;
            switch (edge)
            {
            case Edge::Left:   onBorder = (col == 0); break;
            case Edge::Right:  onBorder = (col == CollisionGrid::NUM_COLS - 1); break;
            case Edge::Top:    onBorder = (row == 0); break;
            case Edge::Bottom: onBorder = (row == CollisionGrid::NUM_ROWS - 1); break;
            default: break;
            }

            if (onBorder) snoBee->Stun(STUN_TIME);
        }
    }

    std::unique_ptr<Component> BorderComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<BorderComponent>(pOwner, m_scene, m_resourceManager);
    }
}
