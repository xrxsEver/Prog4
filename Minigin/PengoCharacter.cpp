#include "PengoCharacter.h"

#include <memory>
#include <glm/vec2.hpp>

#include "InputManager.h"
#include "AddScoreCommand.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"

namespace
{
    constexpr float g_MoveSpeed{200.0f};
}

dae::PengoCharacter::PengoCharacter()
    : Character("Pengo")
{
    InitializeSprite(0.0f, 0.0f);
}

void dae::PengoCharacter::BindKeyboardControls()
{
    auto &input = InputManager::GetInstance();

    input.BindKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{0.0f, -1.0f}, g_MoveSpeed));
    input.BindKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{0.0f, 1.0f}, g_MoveSpeed));
    input.BindKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{-1.0f, 0.0f}, g_MoveSpeed));
    input.BindKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed, std::make_unique<MoveCommand>(this, glm::vec2{1.0f, 0.0f}, g_MoveSpeed));
    input.BindKeyboardCommand(SDL_SCANCODE_C, KeyState::Down, std::make_unique<LoseLifeCommand>(this));
    input.BindKeyboardCommand(SDL_SCANCODE_X, KeyState::Down, std::make_unique<AddScoreCommand>(this, 10));
    input.BindKeyboardCommand(SDL_SCANCODE_V, KeyState::Down, std::make_unique<AddScoreCommand>(this, 100));
}