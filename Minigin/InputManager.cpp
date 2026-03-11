#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <algorithm>
#include <array>
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
	struct KeyboardBinding
	{
		SDL_Scancode key{};
		KeyState triggerState{};
		std::unique_ptr<Command> pCommand{};
	};

	struct GamepadBinding
	{
		std::uint32_t gamepadIndex{};
		Gamepad::Button button{};
		KeyState triggerState{};
		std::unique_ptr<Command> pCommand{};
	};

	std::vector<std::uint8_t> previousKeyboardState{};
	std::vector<std::uint8_t> currentKeyboardState{};
	std::vector<KeyboardBinding> keyboardBindings{};
	std::vector<GamepadBinding> gamepadBindings{};
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
		if (impl.currentKeyboardState.size() != static_cast<size_t>(keyCount))
		{
			impl.currentKeyboardState.assign(static_cast<size_t>(keyCount), 0);
			impl.previousKeyboardState.assign(static_cast<size_t>(keyCount), 0);
		}

		impl.previousKeyboardState = impl.currentKeyboardState;
		for (int i{}; i < keyCount; ++i)
		{
			impl.currentKeyboardState[static_cast<size_t>(i)] = static_cast<std::uint8_t>(pKeyboardState[i] ? 1 : 0);
		}
	}

	for (auto &gamepad : impl.gamepads)
	{
		gamepad.Update();
	}

	for (auto &binding : impl.keyboardBindings)
	{
		const size_t index = static_cast<size_t>(binding.key);
		if (index >= impl.currentKeyboardState.size())
		{
			continue;
		}

		const bool isPressedNow = impl.currentKeyboardState[index] != 0;
		const bool wasPressedLastFrame = impl.previousKeyboardState[index] != 0;
		if (IsKeyTriggered(binding.triggerState, isPressedNow, wasPressedLastFrame) && binding.pCommand)
		{
			binding.pCommand->Execute();
		}
	}

	for (auto &binding : impl.gamepadBindings)
	{
		if (binding.gamepadIndex == AnyGamepad)
		{
			for (auto &gamepad : impl.gamepads)
			{
				if (!gamepad.IsConnected())
				{
					continue;
				}

				if (IsGamepadTriggered(gamepad, binding.button, binding.triggerState) && binding.pCommand)
				{
					binding.pCommand->Execute();
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

		if (IsGamepadTriggered(gamepad, binding.button, binding.triggerState) && binding.pCommand)
		{
			binding.pCommand->Execute();
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

	m_pImpl->keyboardBindings.emplace_back(Impl::KeyboardBinding{key, triggerState, std::move(pCommand)});
}

void dae::InputManager::BindGamepadCommand(const std::uint32_t gamepadIndex, const Gamepad::Button button, const KeyState triggerState, std::unique_ptr<Command> pCommand)
{
	if (!pCommand)
	{
		return;
	}

	m_pImpl->gamepadBindings.emplace_back(Impl::GamepadBinding{gamepadIndex, button, triggerState, std::move(pCommand)});
}

void dae::InputManager::UnbindKeyboardCommand(const SDL_Scancode key, const KeyState triggerState)
{
	auto &bindings = m_pImpl->keyboardBindings;
	bindings.erase(
		std::remove_if(
			bindings.begin(),
			bindings.end(),
			[key, triggerState](const Impl::KeyboardBinding &binding)
			{
				return binding.key == key && binding.triggerState == triggerState;
			}),
		bindings.end());
}

void dae::InputManager::UnbindGamepadCommand(const std::uint32_t gamepadIndex, const Gamepad::Button button, const KeyState triggerState)
{
	auto &bindings = m_pImpl->gamepadBindings;
	bindings.erase(
		std::remove_if(
			bindings.begin(),
			bindings.end(),
			[gamepadIndex, button, triggerState](const Impl::GamepadBinding &binding)
			{
				return binding.gamepadIndex == gamepadIndex && binding.button == button && binding.triggerState == triggerState;
			}),
		bindings.end());
}

void dae::InputManager::ClearBindings()
{
	m_pImpl->keyboardBindings.clear();
	m_pImpl->gamepadBindings.clear();
}

bool dae::InputManager::IsKeyboardPressed(const SDL_Scancode key) const
{
	const auto index = static_cast<size_t>(key);
	return index < m_pImpl->currentKeyboardState.size() && m_pImpl->currentKeyboardState[index] != 0;
}

bool dae::InputManager::IsGamepadConnected(const std::uint32_t gamepadIndex) const
{
	if (gamepadIndex == AnyGamepad)
	{
		for (const auto &gamepad : m_pImpl->gamepads)
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
