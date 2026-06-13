#pragma once
#include <array>
#include <functional>
#include <string>
#include "Component.h"
#include "HighScores.h"

namespace dae
{
    class TextComponent;

    // Arcade name entry: three letter slots the player scrolls through. Up/Down change the letter
    // under the cursor, Left/Right move between slots, and Confirm submits the finished initials.
    // Navigation arrives as Commands (keyboard or gamepad) that call the Move/Change/Confirm hooks,
    // exactly like StartMenuComponent.
    class NameEntryComponent final : public Component
    {
    public:
        using SubmitCallback = std::function<void(const std::string&)>;

        NameEntryComponent(GameObject* pOwner,
                           std::array<TextComponent*, HighScores::kNameLength> slotLabels,
                           SubmitCallback onSubmit);

        void Update(float deltaTime) override;

        // Bound to input.
        void MoveCursor(int delta);   // step between the three slots (wraps)
        void ChangeLetter(int delta); // scroll the current slot's letter (wraps)
        void Confirm();               // hand the finished initials to the callback (once)

        const char* GetDebugName() const override { return "Name Entry"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        void Repaint();
        std::string CurrentName() const;

        std::array<TextComponent*, HighScores::kNameLength> m_slots{};
        SubmitCallback m_onSubmit{};

        std::array<int, HighScores::kNameLength> m_letter{}; // index into m_alphabet per slot
        int m_cursor{ 0 };
        bool m_submitted{ false };

        bool m_dirty{ true };
        float m_blinkTimer{ 0.0f };
        bool m_blinkOn{ true };
    };
}
