#pragma once
#include <SDL3/SDL_scancode.h>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include "Gamepad.h"
#include "Command.h"

namespace dae
{
	enum class KeyState : std::uint8_t
	{
		Down,
		Up,
		Pressed
	};

	class InputManager final
	{
	public:
		static constexpr std::uint32_t AnyGamepad = (std::numeric_limits<std::uint32_t>::max)();

		InputManager();
		~InputManager();

		InputManager(const InputManager &other) = delete;
		InputManager(InputManager &&other) = delete;
		InputManager &operator=(const InputManager &other) = delete;
		InputManager &operator=(InputManager &&other) = delete;

		bool ProcessInput();
		void BindKeyboardCommand(SDL_Scancode key, KeyState triggerState, std::unique_ptr<Command> pCommand);
		void BindGamepadCommand(std::uint32_t gamepadIndex, Gamepad::Button button, KeyState triggerState, std::unique_ptr<Command> pCommand);
		void UnbindKeyboardCommand(SDL_Scancode key, KeyState triggerState);
		void UnbindGamepadCommand(std::uint32_t gamepadIndex, Gamepad::Button button, KeyState triggerState);
		void ClearBindings();
		bool IsKeyboardPressed(SDL_Scancode key) const;
		bool IsGamepadConnected(std::uint32_t gamepadIndex) const;
		std::optional<std::uint32_t> GetFirstConnectedGamepadIndex() const;
		std::optional<Gamepad::StateSnapshot> GetGamepadState(std::uint32_t gamepadIndex) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};

}
