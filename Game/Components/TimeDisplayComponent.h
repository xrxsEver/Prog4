#pragma once
#include <string>
#include "Component.h"

namespace dae
{
    class MazeDrawingComponent;
    class TextComponent;

    // Prints the level's running play time (M:SS), read from the maze coordinator. Clearing the
    // board under 60s pays a time bonus (see MazeDrawingComponent::AwardTimeBonus).
    class TimeDisplayComponent final : public Component
    {
    public:
        TimeDisplayComponent(GameObject* pOwner, MazeDrawingComponent* pMaze, std::string labelPrefix = "TIME");
        ~TimeDisplayComponent() override = default;

        void Update(float deltaTime) override;

        const char* GetDebugName() const override { return "Time Display"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        MazeDrawingComponent* m_pMaze{};
        TextComponent* m_pTextComponent{};
        std::string m_labelPrefix;
        int m_cachedSeconds{ -1 }; // only repaint when the whole-second value changes
    };
}
