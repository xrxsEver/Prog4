#pragma once

#include <cstdint>
#include <memory>

#include "Character.h"
#include "SnoBeeConfig.h"

namespace dae
{
    enum class EnemyState : std::uint8_t
    {
        Idle,
        Walking,
        Dying
    };

    class InputManager;
    class ResourceManager;
    class AnalogStickMoveComponent;

    class SnoBeeCharacter final : public Character
    {
    public:
        explicit SnoBeeCharacter(ResourceManager &resourceManager);
        ~SnoBeeCharacter() override;

        void BindGamepadControls(InputManager &inputManager, std::uint32_t gamepadIndex);

        void ApplyConfig(const SnoBeeConfig& config);
        std::unique_ptr<SnoBeeCharacter> Clone() const;

        static std::unique_ptr<SnoBeeCharacter> Spawn(const SnoBeeCharacter& prototype, const SnoBeeConfig& config);

    private:
        SnoBeeConfig m_currentConfig{};
        AnalogStickMoveComponent* m_pMoveComponent{};
    };
}
