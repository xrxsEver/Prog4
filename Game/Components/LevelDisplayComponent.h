#pragma once
#include <string>
#include "Component.h"

namespace dae
{
    class MazeDrawingComponent;
    class TextComponent;

    // Prints the current level number, read from the maze coordinator
    class LevelDisplayComponent final : public Component
    {
    public:
        LevelDisplayComponent(GameObject* pOwner, MazeDrawingComponent* pMaze, std::string labelPrefix = "LEVEL");
        ~LevelDisplayComponent() override = default;

        void Update(float deltaTime) override;

        const char* GetDebugName() const override { return "Level Display"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        MazeDrawingComponent* m_pMaze{};
        TextComponent* m_pTextComponent{};
        std::string m_labelPrefix;
        int m_cached{ -1 };
    };
}
