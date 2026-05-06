#pragma once
#include "Command.h"
#include "PengoCharacter.h"

namespace dae
{
    // Command to signal a direction key is pressed
    class MoveInputStartCommand final : public Command
    {
    public:
        MoveInputStartCommand(PengoCharacter& pengo, PengoDirection direction)
            : m_pengo(pengo), m_direction(direction) {}

        void Execute() override
        {
            m_pengo.AddMoveInput(m_direction);
        }

    private:
        PengoCharacter& m_pengo;
        PengoDirection m_direction;
    };

    // Command to signal a direction key is released
    class MoveInputStopCommand final : public Command
    {
    public:
        MoveInputStopCommand(PengoCharacter& pengo, PengoDirection direction)
            : m_pengo(pengo), m_direction(direction) {}

        void Execute() override
        {
            m_pengo.RemoveMoveInput(m_direction);
        }

    private:
        PengoCharacter& m_pengo;
        PengoDirection m_direction;
    };
}