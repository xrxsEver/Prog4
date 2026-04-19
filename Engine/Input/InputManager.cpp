#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <array>
#include <cstddef>
#include <span>
#include <unordered_map>
#include <vector>
#include "InputManager.h"

namespace
{
	bool IsKeyTriggered(const dae::KeyState triggerState, const bool isPressedNow, const bool wasPressedLastFrame)
	{
		switch (triggerState)
		{
		case dae::KeyState::Down:
			return isPressedNow && !wasPressedLastFrame;
		case dae::KeyState::Up:
			return !isPressedNow && wasPressedLastFrame;
		case dae::KeyState::Pressed:
			return isPressedNow;
		default:
			return false;
		}
	}

	bool IsGamepadTriggered(const dae::Gamepad &gamepad, const dae::Gamepad::Button button, const dae::KeyState triggerState)
	{
		switch (triggerState)
		{
		case dae::KeyState::Down:
			return gamepad.IsDownThisFrame(button);
		case dae::KeyState::Up:
			return gamepad.IsUpThisFrame(button);
		case dae::KeyState::Pressed:
			return gamepad.IsPressed(button);
		default:
			return false;
		}
	}
}

struct dae::InputManager::Impl
{
	struct KeyboardBindingKey
	{
		SDL_Scancode key{};
		KeyState triggerState{};

		bool operator==(const KeyboardBindingKey &other) const = default;
	};

	struct GamepadBindingKey
	{
		std::uint32_t gamepadIndex{};
		Gamepad::Button button{};
		KeyState triggerState{};

		bool operator==(const GamepadBindingKey &other) const = default;
	};

	struct KeyboardBindingKeyHasher
	{
		std::size_t operator()(const KeyboardBindingKey &key) const noexcept
		{
			const std::size_t keyHash = std::hash<int>{}(static_cast<int>(key.key));
			const std::size_t triggerHash = std::hash<std::uint8_t>{}(static_cast<std::uint8_t>(key.triggerState));
			return keyHash ^ (triggerHash << 1);
		}
	};

	struct GamepadBindingKeyHasher
	{
		std::size_t operator()(const GamepadBindingKey &key) const noexcept
		{
			const std::size_t indexHash = std::hash<std::uint32_t>{}(key.gamepadIndex);
			const std::size_t buttonHash = std::hash<std::uint16_t>{}(static_cast<std::uint16_t>(key.button));
			const std::size_t triggerHash = std::hash<std::uint8_t>{}(static_cast<std::uint8_t>(key.triggerState));
			return indexHash ^ (buttonHash << 1) ^ (triggerHash << 2);
		}
	};

	std::vector<std::uint8_t> previousKeyboardState{};
	std::vector<std::uint8_t> currentKeyboardState{};
	std::unordered_map<KeyboardBindingKey, std::unique_ptr<Command>, KeyboardBindingKeyHasher> keyboardBindings{};
	std::unordered_map<GamepadBindingKey, std::unique_ptr<Command>, GamepadBindingKeyHasher> gamepadBindings{};
	std::array<Gamepad, 4> gamepads{Gamepad{0}, Gamepad{1}, Gamepad{2}, Gamepad{3}};
};

dae::InputManager::InputManager()
	: m_pImpl(std::make_unique<Impl>())
{
}

dae::InputManager::~InputManager() = default;

bool dae::InputManager::ProcessInput()
{
	auto &impl = *m_pImpl;
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
		{
			return false;
		}

		// process event for IMGUI
		ImGui_ImplSDL3_ProcessEvent(&e);
	}

	// keep keyboard state arrays in sync with SDL's scancode array
	SDL_PumpEvents();
	int keyCount{};
	const bool *const pKeyboardState = SDL_GetKeyboardState(&keyCount);
	if (pKeyboardState != nullptr && keyCount > 0)
	{
		const std::span<const bool> keyboardState{pKeyboardState, static_cast<std::size_t>(keyCount)};
		if (impl.currentKeyboardState.size() != static_cast<std::size_t>(keyCount))
		{
			impl.currentKeyboardState.assign(static_cast<std::size_t>(keyCount), 0);
			impl.previousKeyboardState.assign(static_cast<std::size_t>(keyCount), 0);
		}

		impl.previousKeyboardState = impl.currentKeyboardState;
		for (std::size_t i{}; i < keyboardState.size(); ++i)
		{
			impl.currentKeyboardState[i] = static_cast<std::uint8_t>(keyboardState[i] ? 1 : 0);
		}
	}

	for (auto &gamepad : std::span<Gamepad>{impl.gamepads})
	{
		gamepad.Update();
	}

	for (const auto &[binding, command] : impl.keyboardBindings)
	{
		const std::size_t index = static_cast<std::size_t>(binding.key);
		if (index >= impl.currentKeyboardState.size())
		{
			continue;
		}

		const bool isPressedNow = impl.currentKeyboardState[index] != 0;
		const bool wasPressedLastFrame = impl.previousKeyboardState[index] != 0;
		if (IsKeyTriggered(binding.triggerState, isPressedNow, wasPressedLastFrame) && command)
		{
			command->Execute();
		}
	}

	for (const auto &[binding, command] : impl.gamepadBindings)
	{
		if (binding.gamepadIndex == AnyGamepad)
		{
			for (auto &gamepad : std::span<Gamepad>{impl.gamepads})
			{
				if (!gamepad.IsConnected())
				{
					continue;
				}

				if (IsGamepadTriggered(gamepad, binding.button, binding.triggerState) && command)
				{
					command->Execute();
					break;
				}
			}
			continue;
		}

		if (binding.gamepadIndex >= impl.gamepads.size())
		{
			continue;
		}

		auto &gamepad = impl.gamepads[binding.gamepadIndex];
		if (!gamepad.IsConnected())
		{
			continue;
		}

		if (IsGamepadTriggered(gamepad, binding.button, binding.triggerState) && command)
		{
			command->Execute();
		}
	}

	return true;
}

void dae::InputManager::BindKeyboardCommand(const SDL_Scancode key, const KeyState triggerState, std::unique_ptr<Command> pCommand)
{
	if (!pCommand)
	{
		return;
	}

	m_pImpl->keyboardBindings.insert_or_assign(Impl::KeyboardBindingKey{key, triggerState}, std::move(pCommand));
}

void dae::InputManager::BindGamepadCommand(const std::uint32_t gamepadIndex, const Gamepad::Button button, const KeyState triggerState, std::unique_ptr<Command> pCommand)
{
	if (!pCommand)
	{
		return;
	}

	m_pImpl->gamepadBindings.insert_or_assign(Impl::GamepadBindingKey{gamepadIndex, button, triggerState}, std::move(pCommand));
}

void dae::InputManager::UnbindKeyboardCommand(const SDL_Scancode key, const KeyState triggerState)
{
	m_pImpl->keyboardBindings.erase(Impl::KeyboardBindingKey{key, triggerState});
}

void dae::InputManager::UnbindGamepadCommand(const std::uint32_t gamepadIndex, const Gamepad::Button button, const KeyState triggerState)
{
	m_pImpl->gamepadBindings.erase(Impl::GamepadBindingKey{gamepadIndex, button, triggerState});
}

void dae::InputManager::ClearBindings()
{
	m_pImpl->keyboardBindings.clear();
	m_pImpl->gamepadBindings.clear();
}

bool dae::InputManager::IsKeyboardPressed(const SDL_Scancode key) const
{
	const auto index = static_cast<std::size_t>(key);
	return index < m_pImpl->currentKeyboardState.size() && m_pImpl->currentKeyboardState[index] != 0;
}

bool dae::InputManager::IsGamepadConnected(const std::uint32_t gamepadIndex) const
{
	if (gamepadIndex == AnyGamepad)
	{
		for (const auto &gamepad : std::span<const Gamepad>{m_pImpl->gamepads})
		{
			if (gamepad.IsConnected())
			{
				return true;
			}
		}
		return false;
	}

	return gamepadIndex < m_pImpl->gamepads.size() && m_pImpl->gamepads[gamepadIndex].IsConnected();
}

std::optional<std::uint32_t> dae::InputManager::GetFirstConnectedGamepadIndex() const
{
	for (std::uint32_t index{}; index < m_pImpl->gamepads.size(); ++index)
	{
		if (m_pImpl->gamepads[index].IsConnected())
		{
			return index;
		}
	}

	return std::nullopt;
}

std::optional<dae::Gamepad::StateSnapshot> dae::InputManager::GetGamepadState(const std::uint32_t gamepadIndex) const
{
	if (gamepadIndex == AnyGamepad)
	{
		if (const auto connectedIndex = GetFirstConnectedGamepadIndex())
		{
			return m_pImpl->gamepads[*connectedIndex].GetStateSnapshot();
		}

		return std::nullopt;
	}

	if (gamepadIndex >= m_pImpl->gamepads.size() || !m_pImpl->gamepads[gamepadIndex].IsConnected())
	{
		return std::nullopt;
	}

	return m_pImpl->gamepads[gamepadIndex].GetStateSnapshot();
}
