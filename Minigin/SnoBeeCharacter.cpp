#include "SnoBeeCharacter.h"

#include <memory>
#include <glm/vec2.hpp>

#include "AnalogStickMoveComponent.h"
#include "AddScoreCommand.h"
#include "InputManager.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"

namespace
{
    constexpr float g_MoveSpeed{400.0f};
}

dae::SnoBeeCharacter::SnoBeeCharacter()
    : Character("SnoBee")
{
    InitializeSprite(8 * 16.0f, 9 * 16.0f);
}

void dae::SnoBeeCharacter::BindGamepadControls(const std::uint32_t gamepadIndex)
{
    auto &input = InputManager::GetInstance();

    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadUp, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{0.0f, -1.0f}, g_MoveSpeed));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadDown, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{0.0f, 1.0f}, g_MoveSpeed));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadLeft, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{-1.0f, 0.0f}, g_MoveSpeed));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadRight, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{1.0f, 0.0f}, g_MoveSpeed));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::X, KeyState::Down, std::make_unique<LoseLifeCommand>(this));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::A, KeyState::Down, std::make_unique<AddScoreCommand>(this, 10));
    input.BindGamepadCommand(gamepadIndex, Gamepad::Button::B, KeyState::Down, std::make_unique<AddScoreCommand>(this, 100));
    AddComponent<AnalogStickMoveComponent>(gamepadIndex, g_MoveSpeed);
}