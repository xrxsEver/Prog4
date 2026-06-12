#pragma once
#include "Component.h"
#include <memory>

namespace dae
{
    class Scene;
    class ResourceManager;
    class Texture2D;

    // The field border. Pushing straight into a wall makes that edge rattle (a quick
    // two-frame flicker for a second) and stuns every Sno-Bee pinned against it.
    class BorderComponent final : public Component
    {
    public:
        enum class Edge { None, Left, Right, Top, Bottom };

        BorderComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager);
        ~BorderComponent() override = default;

        void Update(float deltaTime) override;
        void Render() const override;

        // Pengo bumped this edge: start the vibration and stun whoever is touching it
        void Push(Edge edge);

        const char* GetDebugName() const override { return "Border Component"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        void StunSnoBeesAlong(Edge edge) const;

        Scene& m_scene;
        ResourceManager& m_resourceManager;
        std::shared_ptr<Texture2D> m_borderTexture;

        Edge m_activeEdge{ Edge::None };
        float m_timer{ 0.0f };       // seconds left in the current vibration
        float m_flashTimer{ 0.0f };
        bool m_secondFrame{ false }; // which of the two frames is showing

        static constexpr float VIBRATE_DURATION = 1.0f; // how long an edge rattles per push
        static constexpr float FLASH_INTERVAL = 0.08f;  // fast flip between the two frames
        static constexpr float STUN_TIME = 3.0f;        // freeze for Sno-Bees caught on the edge
        static constexpr float THICKNESS = 16.0f;       // matches the playfield wall (8px art at 2x)
    };
}
