#pragma once
#include <cstdint>
#include <memory>
#include <string_view>

namespace dae
{
    class Gamepad final
    {
    public:
        enum class Button : std::uint16_t
        {
            DPadUp = 0x0001,
            DPadDown = 0x0002,
            DPadLeft = 0x0004,
            DPadRight = 0x0008,
            Start = 0x0010,
            Back = 0x0020,
            LeftThumb = 0x0040,
            RightThumb = 0x0080,
            LeftShoulder = 0x0100,
            RightShoulder = 0x0200,
            A = 0x1000,
            B = 0x2000,
            X = 0x4000,
            Y = 0x8000
        };

        struct StateSnapshot
        {
            std::uint32_t playerIndex{};
            std::uint32_t packetNumber{};
            std::uint16_t buttons{};
            std::uint8_t leftTrigger{};
            std::uint8_t rightTrigger{};
            std::int16_t leftThumbX{};
            std::int16_t leftThumbY{};
            std::int16_t rightThumbX{};
            std::int16_t rightThumbY{};
            std::int16_t leftDeadzone{};
            std::int16_t rightDeadzone{};
            std::uint8_t triggerThreshold{};
            bool connected{};
            std::string_view backend{};
        };

        explicit Gamepad(std::uint32_t playerIndex);
        ~Gamepad();

        Gamepad(const Gamepad &other) = delete;
        Gamepad(Gamepad &&other) noexcept;
        Gamepad &operator=(const Gamepad &other) = delete;
        Gamepad &operator=(Gamepad &&other) noexcept;

        void Update();
        bool IsConnected() const;
        bool IsDownThisFrame(Button button) const;
        bool IsUpThisFrame(Button button) const;
        bool IsPressed(Button button) const;
        StateSnapshot GetStateSnapshot() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}
