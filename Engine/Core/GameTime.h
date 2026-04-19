#pragma once
#include "Singleton.h"

namespace dae
{
    class GameTime final : public Singleton<GameTime>
    {
    public:
        float GetDeltaTime() const { return m_deltaTime; }
        void SetDeltaTime(float dt) { m_deltaTime = dt; }

        static constexpr float GetFixedTimeStep() { return m_fixedTimeStep; }
        static constexpr float GetMaxDeltaTime() { return m_maxDeltaTime; }

    private:
        friend class Singleton<GameTime>;
        GameTime() = default;

        float m_deltaTime{};
        static constexpr float m_fixedTimeStep{0.02f}; // 50 Hz, like Unity
        static constexpr float m_maxDeltaTime{0.25f};  // prevent large frame spikes from destabilizing updates
    };
}
