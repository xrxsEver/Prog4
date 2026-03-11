#include "Gamepad.h"

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#else
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_stdinc.h>
#endif

#include <utility>

namespace
{
#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
    constexpr std::int16_t g_LeftStickDeadzone{XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE};
    constexpr std::int16_t g_RightStickDeadzone{XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE};
    constexpr std::uint8_t g_TriggerThreshold{XINPUT_GAMEPAD_TRIGGER_THRESHOLD};
    constexpr std::string_view g_BackendName{"XInput"};
#else
    constexpr std::int16_t g_LeftStickDeadzone{7849};
    constexpr std::int16_t g_RightStickDeadzone{8689};
    constexpr std::uint8_t g_TriggerThreshold{30};
    constexpr std::string_view g_BackendName{"SDL Gamepad"};

    std::uint16_t BuildButtonMask(SDL_Gamepad *gamepad)
    {
        if (gamepad == nullptr)
        {
            return 0;
        }

        std::uint16_t buttonMask{};
        const auto appendButton = [&buttonMask, gamepad](const dae::Gamepad::Button button, const SDL_GamepadButton sdlButton)
        {
            if (SDL_GetGamepadButton(gamepad, sdlButton))
            {
                buttonMask = static_cast<std::uint16_t>(buttonMask | static_cast<std::uint16_t>(button));
            }
        };

        appendButton(dae::Gamepad::Button::DPadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);
        appendButton(dae::Gamepad::Button::DPadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        appendButton(dae::Gamepad::Button::DPadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        appendButton(dae::Gamepad::Button::DPadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        appendButton(dae::Gamepad::Button::Start, SDL_GAMEPAD_BUTTON_START);
        appendButton(dae::Gamepad::Button::Back, SDL_GAMEPAD_BUTTON_BACK);
        appendButton(dae::Gamepad::Button::LeftThumb, SDL_GAMEPAD_BUTTON_LEFT_STICK);
        appendButton(dae::Gamepad::Button::RightThumb, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
        appendButton(dae::Gamepad::Button::LeftShoulder, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
        appendButton(dae::Gamepad::Button::RightShoulder, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
        appendButton(dae::Gamepad::Button::A, SDL_GAMEPAD_BUTTON_SOUTH);
        appendButton(dae::Gamepad::Button::B, SDL_GAMEPAD_BUTTON_EAST);
        appendButton(dae::Gamepad::Button::X, SDL_GAMEPAD_BUTTON_WEST);
        appendButton(dae::Gamepad::Button::Y, SDL_GAMEPAD_BUTTON_NORTH);

        return buttonMask;
    }

    std::uint8_t ReadTrigger(SDL_Gamepad *gamepad, const SDL_GamepadAxis axis)
    {
        if (gamepad == nullptr)
        {
            return 0;
        }

        const auto rawValue = SDL_GetGamepadAxis(gamepad, axis);
        if (rawValue <= 0)
        {
            return 0;
        }

        return static_cast<std::uint8_t>((static_cast<std::uint32_t>(rawValue) * 255u) / 32767u);
    }
#endif
}

struct dae::Gamepad::Impl
{
#if !defined(_WIN32) || defined(__EMSCRIPTEN__)
    ~Impl()
    {
        if (handle != nullptr)
        {
            SDL_CloseGamepad(handle);
            handle = nullptr;
        }
    }
#endif

    std::uint32_t playerIndex{};
    std::uint32_t packetNumber{};
    std::uint16_t currentButtons{};
    std::uint16_t previousButtons{};
    std::uint16_t buttonsDownThisFrame{};
    std::uint16_t buttonsUpThisFrame{};
    std::uint8_t leftTrigger{};
    std::uint8_t rightTrigger{};
    std::int16_t leftThumbX{};
    std::int16_t leftThumbY{};
    std::int16_t rightThumbX{};
    std::int16_t rightThumbY{};
    std::int16_t leftDeadzone{g_LeftStickDeadzone};
    std::int16_t rightDeadzone{g_RightStickDeadzone};
    std::uint8_t triggerThreshold{g_TriggerThreshold};
    bool connected{};

#if !defined(_WIN32) || defined(__EMSCRIPTEN__)
    SDL_JoystickID instanceId{};
    SDL_Gamepad *handle{};
#endif
};

dae::Gamepad::Gamepad(const std::uint32_t playerIndex)
    : m_pImpl(std::make_unique<Impl>())
{
    m_pImpl->playerIndex = playerIndex;
}

dae::Gamepad::~Gamepad() = default;

dae::Gamepad::Gamepad(Gamepad &&other) noexcept = default;
dae::Gamepad &dae::Gamepad::operator=(Gamepad &&other) noexcept = default;

void dae::Gamepad::Update()
{
    auto &impl = *m_pImpl;
    impl.previousButtons = impl.currentButtons;

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
    XINPUT_STATE state{};
    const DWORD result = XInputGetState(impl.playerIndex, &state);

    if (result == ERROR_SUCCESS)
    {
        impl.connected = true;
        impl.packetNumber = state.dwPacketNumber;
        impl.currentButtons = state.Gamepad.wButtons;
        impl.leftTrigger = state.Gamepad.bLeftTrigger;
        impl.rightTrigger = state.Gamepad.bRightTrigger;
        impl.leftThumbX = state.Gamepad.sThumbLX;
        impl.leftThumbY = state.Gamepad.sThumbLY;
        impl.rightThumbX = state.Gamepad.sThumbRX;
        impl.rightThumbY = state.Gamepad.sThumbRY;
    }
    else
    {
        impl.connected = false;
        impl.packetNumber = 0;
        impl.currentButtons = 0;
        impl.leftTrigger = 0;
        impl.rightTrigger = 0;
        impl.leftThumbX = 0;
        impl.leftThumbY = 0;
        impl.rightThumbX = 0;
        impl.rightThumbY = 0;
    }
#else
    SDL_JoystickID desiredInstanceId{};
    bool foundDesiredGamepad{};
    int gamepadCount{};
    SDL_JoystickID *gamepadIds = SDL_GetGamepads(&gamepadCount);
    if (gamepadIds != nullptr && impl.playerIndex < static_cast<std::uint32_t>(gamepadCount))
    {
        desiredInstanceId = gamepadIds[impl.playerIndex];
        foundDesiredGamepad = true;
    }
    SDL_free(gamepadIds);

    const bool slotChanged = !foundDesiredGamepad || desiredInstanceId != impl.instanceId;
    const bool handleDisconnected = impl.handle != nullptr && !SDL_GamepadConnected(impl.handle);
    if (slotChanged || handleDisconnected)
    {
        if (impl.handle != nullptr)
        {
            SDL_CloseGamepad(impl.handle);
            impl.handle = nullptr;
        }

        impl.instanceId = desiredInstanceId;
        if (foundDesiredGamepad)
        {
            impl.handle = SDL_OpenGamepad(desiredInstanceId);
        }
    }

    if (impl.handle != nullptr && SDL_GamepadConnected(impl.handle))
    {
        const auto previousPacket = impl.packetNumber;
        const auto previousLeftTrigger = impl.leftTrigger;
        const auto previousRightTrigger = impl.rightTrigger;
        const auto previousLeftThumbX = impl.leftThumbX;
        const auto previousLeftThumbY = impl.leftThumbY;
        const auto previousRightThumbX = impl.rightThumbX;
        const auto previousRightThumbY = impl.rightThumbY;

        impl.connected = true;
        impl.currentButtons = BuildButtonMask(impl.handle);
        impl.leftTrigger = ReadTrigger(impl.handle, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        impl.rightTrigger = ReadTrigger(impl.handle, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
        impl.leftThumbX = SDL_GetGamepadAxis(impl.handle, SDL_GAMEPAD_AXIS_LEFTX);
        impl.leftThumbY = SDL_GetGamepadAxis(impl.handle, SDL_GAMEPAD_AXIS_LEFTY);
        impl.rightThumbX = SDL_GetGamepadAxis(impl.handle, SDL_GAMEPAD_AXIS_RIGHTX);
        impl.rightThumbY = SDL_GetGamepadAxis(impl.handle, SDL_GAMEPAD_AXIS_RIGHTY);

        if (previousPacket == 0 ||
            impl.currentButtons != impl.previousButtons ||
            impl.leftTrigger != previousLeftTrigger ||
            impl.rightTrigger != previousRightTrigger ||
            impl.leftThumbX != previousLeftThumbX ||
            impl.leftThumbY != previousLeftThumbY ||
            impl.rightThumbX != previousRightThumbX ||
            impl.rightThumbY != previousRightThumbY)
        {
            ++impl.packetNumber;
        }
    }
    else
    {
        impl.connected = false;
        impl.packetNumber = 0;
        impl.currentButtons = 0;
        impl.leftTrigger = 0;
        impl.rightTrigger = 0;
        impl.leftThumbX = 0;
        impl.leftThumbY = 0;
        impl.rightThumbX = 0;
        impl.rightThumbY = 0;
    }
#endif

    impl.buttonsDownThisFrame = static_cast<std::uint16_t>((~impl.previousButtons) & impl.currentButtons);
    impl.buttonsUpThisFrame = static_cast<std::uint16_t>(impl.previousButtons & (~impl.currentButtons));
}

bool dae::Gamepad::IsConnected() const
{
    return m_pImpl->connected;
}

bool dae::Gamepad::IsDownThisFrame(const Button button) const
{
    return (m_pImpl->buttonsDownThisFrame & static_cast<std::uint16_t>(button)) != 0;
}

bool dae::Gamepad::IsUpThisFrame(const Button button) const
{
    return (m_pImpl->buttonsUpThisFrame & static_cast<std::uint16_t>(button)) != 0;
}

bool dae::Gamepad::IsPressed(const Button button) const
{
    return (m_pImpl->currentButtons & static_cast<std::uint16_t>(button)) != 0;
}

dae::Gamepad::StateSnapshot dae::Gamepad::GetStateSnapshot() const
{
    const auto &impl = *m_pImpl;
    return StateSnapshot{
        impl.playerIndex,
        impl.packetNumber,
        impl.currentButtons,
        impl.leftTrigger,
        impl.rightTrigger,
        impl.leftThumbX,
        impl.leftThumbY,
        impl.rightThumbX,
        impl.rightThumbY,
        impl.leftDeadzone,
        impl.rightDeadzone,
        impl.triggerThreshold,
        impl.connected,
        g_BackendName};
}
