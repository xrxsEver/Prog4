#pragma once
#include <array>
#include <string>
#include <functional>
#include "Component.h"
#include "GameMode.h"

namespace dae
{
    class InputManager;
    class TextComponent;

    // Drives the start-menu rows: tracks the highlighted option, greys out the two-player
    // modes until a controller is connected, and reports the chosen mode through a callback.
    // Navigation/confirmation arrive as Commands (keyboard or gamepad) that call Move/Confirm.
    class StartMenuComponent final : public Component
    {
    public:
        using SelectCallback = std::function<void(GameMode)>;

        StartMenuComponent(GameObject* pOwner, InputManager& inputManager,
                           std::array<TextComponent*, kGameModeCount> rowLabels,
                           SelectCallback onSelect);

        void Update(float deltaTime) override;

        // Bound to input: step the cursor (skipping disabled rows) and activate a row.
        void Move(int delta);
        void Confirm();

        // Re-evaluate which rows are selectable and repaint the labels. Called when the menu
        // becomes visible again so the grey-out reflects the controllers connected right now.
        void RefreshNow() { m_dirty = true; }

        const char* GetDebugName() const override { return "Start Menu"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        void RecomputeEnabled();
        void RepaintLabels();

        InputManager& m_inputManager;
        std::array<TextComponent*, kGameModeCount> m_rowLabels{};
        SelectCallback m_onSelect{};

        std::array<std::string, kGameModeCount> m_rowText{ "SINGLE PLAYER", "CO-OP", "VERSUS" };
        std::array<bool, kGameModeCount> m_enabled{ true, false, false };

        int m_selected{ 0 };
        bool m_dirty{ true };          // labels need a repaint
        int m_lastSelected{ -1 };      // last painted state, to avoid needless TTF re-renders
        std::array<bool, kGameModeCount> m_lastEnabled{ false, false, false };
    };
}
