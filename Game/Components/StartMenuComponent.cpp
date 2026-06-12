#include "StartMenuComponent.h"
#include "TextComponent.h"
#include "InputManager.h"

namespace dae
{
    namespace
    {
        constexpr TextComponent::Color kWhite{ 255, 255, 255, 255 };
        constexpr TextComponent::Color kYellow{ 255, 209, 0, 255 };
        constexpr TextComponent::Color kGrey{ 90, 90, 90, 255 };
    }

    StartMenuComponent::StartMenuComponent(GameObject* pOwner, InputManager& inputManager,
                                           std::array<TextComponent*, kGameModeCount> rowLabels,
                                           SelectCallback onSelect)
        : Component(pOwner)
        , m_inputManager(inputManager)
        , m_rowLabels(rowLabels)
        , m_onSelect(std::move(onSelect))
    {
    }

    void StartMenuComponent::RecomputeEnabled()
    {
        // Two-player modes need a controller for the second player; single player never does.
        const bool hasController = m_inputManager.IsGamepadConnected(InputManager::AnyGamepad);
        m_enabled[0] = true;
        m_enabled[1] = hasController;
        m_enabled[2] = hasController;

        // If the highlighted row just got disabled (controller unplugged), fall back to single player.
        if (!m_enabled[m_selected])
        {
            m_selected = 0;
        }
    }

    void StartMenuComponent::Update(float /*deltaTime*/)
    {
        RecomputeEnabled();

        if (m_dirty || m_selected != m_lastSelected || m_enabled != m_lastEnabled)
        {
            RepaintLabels();
            m_lastSelected = m_selected;
            m_lastEnabled = m_enabled;
            m_dirty = false;
        }
    }

    void StartMenuComponent::Move(int delta)
    {
        if (delta == 0)
        {
            return;
        }

        // Walk in the requested direction until we land on an enabled row (or give up after a lap).
        int index = m_selected;
        for (int step = 0; step < kGameModeCount; ++step)
        {
            index = (index + delta + kGameModeCount) % kGameModeCount;
            if (m_enabled[index])
            {
                m_selected = index;
                break;
            }
        }
    }

    void StartMenuComponent::Confirm()
    {
        if (m_enabled[m_selected] && m_onSelect)
        {
            m_onSelect(static_cast<GameMode>(m_selected));
        }
    }

    void StartMenuComponent::RepaintLabels()
    {
        for (int i = 0; i < kGameModeCount; ++i)
        {
            if (m_rowLabels[i] == nullptr)
            {
                continue;
            }

            const bool selected = (i == m_selected);
            m_rowLabels[i]->SetText((selected ? "> " : "  ") + m_rowText[i]);
            m_rowLabels[i]->SetColor(!m_enabled[i] ? kGrey : (selected ? kYellow : kWhite));
        }
    }

    std::unique_ptr<Component> StartMenuComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<StartMenuComponent>(pOwner, m_inputManager, m_rowLabels, m_onSelect);
    }
}
