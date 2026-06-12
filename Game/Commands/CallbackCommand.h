#pragma once
#include <functional>
#include <utility>
#include "Command.h"

namespace dae
{
    // A Command that simply runs a std::function. Handy for one-off UI actions (e.g. returning
    // to the menu) where writing a dedicated command class would be overkill.
    class CallbackCommand final : public Command
    {
    public:
        explicit CallbackCommand(std::function<void()> action)
            : m_action(std::move(action)) {}

        void Execute() override
        {
            if (m_action)
            {
                m_action();
            }
        }

    private:
        std::function<void()> m_action;
    };
}
