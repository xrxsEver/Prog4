#pragma once
#include <memory>
#include "Component.h"

namespace dae
{
    class MazeDrawingComponent;
    class ResourceManager;
    class Texture2D;

    // Shows the Sno-Bees still waiting in reserve as a stack of little circles on the right,
    // flashing between two frames so they read as "alive". Reads its count from the maze coordinator.
    class SnoBeeCounterComponent final : public Component
    {
    public:
        SnoBeeCounterComponent(GameObject* pOwner, ResourceManager& resourceManager, MazeDrawingComponent* pMaze);

        void Update(float deltaTime) override;
        void Render() const override;

        const char* GetDebugName() const override { return "Sno-Bee Counter Display"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        ResourceManager* m_pResourceManager{};
        MazeDrawingComponent* m_pMaze{};
        std::shared_ptr<Texture2D> m_miscTexture{};

        float m_flashTimer{0.0f};
        bool m_flashOn{false};

        // The two 8x8 circles sit in the bottom-right corner of misc.png
        static constexpr float ICON_SOURCE_X = 128.0f;
        static constexpr float ICON_SOURCE_Y = 88.0f;
        static constexpr float ICON_SOURCE_SIZE = 8.0f;
        static constexpr float ICON_FRAME_STRIDE = 8.0f; // distance to the second frame

        static constexpr float ICON_DRAW_SIZE = 20.0f;
        static constexpr float ICON_SPACING = 6.0f;
        static constexpr float FLASH_INTERVAL = 0.5f;
    };
}
