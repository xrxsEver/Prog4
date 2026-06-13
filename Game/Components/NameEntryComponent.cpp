#include "NameEntryComponent.h"
#include "TextComponent.h"

namespace dae
{
    namespace
    {
        // No spaces, so a saved "<score> <name>" line parses back cleanly. Letters first, then a
        // couple of punctuation marks the way the old cabinets allowed.
        const std::string kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.-";

        constexpr TextComponent::Color kWhite{ 255, 255, 255, 255 };
        constexpr TextComponent::Color kYellow{ 255, 209, 0, 255 };
        constexpr TextComponent::Color kDim{ 110, 95, 0, 255 };

        constexpr float kBlinkInterval = 0.4f; // active slot blink rate (seconds)
    }

    NameEntryComponent::NameEntryComponent(GameObject* pOwner,
                                           std::array<TextComponent*, HighScores::kNameLength> slotLabels,
                                           SubmitCallback onSubmit)
        : Component(pOwner)
        , m_slots(slotLabels)
        , m_onSubmit(std::move(onSubmit))
    {
    }

    void NameEntryComponent::Update(float deltaTime)
    {
        // Blink the slot under the cursor so it reads as "the one you're editing".
        m_blinkTimer += deltaTime;
        if (m_blinkTimer >= kBlinkInterval)
        {
            m_blinkTimer -= kBlinkInterval;
            m_blinkOn = !m_blinkOn;
            m_dirty = true;
        }

        if (m_dirty)
        {
            Repaint();
            m_dirty = false;
        }
    }

    void NameEntryComponent::MoveCursor(int delta)
    {
        if (delta == 0 || m_submitted)
        {
            return;
        }
        const int n = HighScores::kNameLength;
        m_cursor = (m_cursor + delta % n + n) % n;
        m_blinkOn = true;          // snap bright on a move so the new slot is obvious
        m_blinkTimer = 0.0f;
        m_dirty = true;
    }

    void NameEntryComponent::ChangeLetter(int delta)
    {
        if (delta == 0 || m_submitted)
        {
            return;
        }
        const int n = static_cast<int>(kAlphabet.size());
        m_letter[m_cursor] = (m_letter[m_cursor] + delta % n + n) % n;
        m_dirty = true;
    }

    void NameEntryComponent::Confirm()
    {
        if (m_submitted)
        {
            return;
        }
        m_submitted = true;
        if (m_onSubmit)
        {
            m_onSubmit(CurrentName());
        }
    }

    std::string NameEntryComponent::CurrentName() const
    {
        std::string name;
        name.reserve(HighScores::kNameLength);
        for (const int idx : m_letter)
        {
            name.push_back(kAlphabet[static_cast<std::size_t>(idx)]);
        }
        return name;
    }

    void NameEntryComponent::Repaint()
    {
        for (int i = 0; i < HighScores::kNameLength; ++i)
        {
            if (m_slots[i] == nullptr)
            {
                continue;
            }

            m_slots[i]->SetText(std::string(1, kAlphabet[static_cast<std::size_t>(m_letter[i])]));

            const bool active = (i == m_cursor);
            m_slots[i]->SetColor(active ? (m_blinkOn ? kYellow : kDim) : kWhite);
        }
    }

    std::unique_ptr<Component> NameEntryComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<NameEntryComponent>(pOwner, m_slots, m_onSubmit);
    }
}
