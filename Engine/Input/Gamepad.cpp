#include "Gamepad.h"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_stdinc.h>

#include <string_view>
#include <utility>
#include <memory>

// We drive every platform through SDL's gamepad API rather than XInput. XInput only sees
// Xbox-class pads, so a DualShock/DualSense (or any non-XInput controller) was invisible on
// Windows; SDL's layer recognises Xbox, PlayStation, Bluetooth and generic pads out of the box
// (built-in mapping db) and gives us hot-plug for free, which is what two-player co-op/versus needs.
namespace
{
    constexpr std::int16_t g_LeftStickDeadzone{7849};
    constexpr std::int16_t g_RightStickDeadzone{8689};
    constexpr std::uint8_t g_TriggerThreshold{30};
    constexpr std::string_view g_BackendName{"SDL Gamepad"};

    struct SDLJoystickIdsDeleter final
    {
        void operator()(SDL_JoystickID *ids) const noexcept
        {
            SDL_free(ids);
        }
    };

    using SDLJoystickIdsPtr = std::unique_ptr<SDL_JoystickID, SDLJoystickIdsDeleter>;

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
}

struct dae::Gamepad::Impl
{
    ~Impl()
    {
        if (handle != nullptr)
        {
            SDL_CloseGamepad(handle);
            handle = nullptr;
        }
    }

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

    SDL_JoystickID instanceId{};
    SDL_Gamepad *handle{};
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

    // The Nth connected gamepad in SDL's list is "player N"; this keeps a stable mapping and
    // re-opens the handle whenever that slot changes (a pad was plugged in or pulled out).
    SDL_JoystickID desiredInstanceId{};
    bool foundDesiredGamepad{};
    int gamepadCount{};
    SDLJoystickIdsPtr gamepadIds{SDL_GetGamepads(&gamepadCount)};
    if (gamepadIds != nullptr && impl.playerIndex < static_cast<std::uint32_t>(gamepadCount))
    {
        desiredInstanceId = gamepadIds.get()[impl.playerIndex];
        foundDesiredGamepad = true;
    }

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
