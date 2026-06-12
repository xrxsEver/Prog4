#pragma once
#include "Command.h"
#include "StartMenuComponent.h"

namespace dae
{
    // Move the menu cursor by a fixed step (skipping disabled rows is the menu's job).
    class MenuNavigateCommand final : public Command
    {
    public:
        MenuNavigateCommand(StartMenuComponent& menu, int delta)
            : m_pMenu(&menu), m_delta(delta) {}

        void Execute() override { m_pMenu->Move(m_delta); }

    private:
        StartMenuComponent* m_pMenu{};
        int m_delta{};
    };

    // Activate the highlighted row.
    class MenuConfirmCommand final : public Command
    {
    public:
        explicit MenuConfirmCommand(StartMenuComponent& menu)
            : m_pMenu(&menu) {}

        void Execute() override { m_pMenu->Confirm(); }

    private:
        StartMenuComponent* m_pMenu{};
    };
}
