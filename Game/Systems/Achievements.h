#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "Observer.h"

namespace dae
{
    class Character;

    class Achievements final : public Observer
    {
    public:
        Achievements();
        ~Achievements();

        static Achievements *GetActiveInstance();

        Achievements(const Achievements &other) = delete;
        Achievements(Achievements &&other) = delete;
        Achievements &operator=(const Achievements &other) = delete;
        Achievements &operator=(Achievements &&other) = delete;

        void Update();
        void OnNotify(GameEvent event) override;
        [[nodiscard]] bool IsSteamAvailable() const;
        [[nodiscard]] std::uint32_t GetCurrentAppId() const;
        [[nodiscard]] bool IsSpacewarAppIdActive() const;
        [[nodiscard]] bool Unlock(std::string_view achievementId) const;
        [[nodiscard]] bool ClearSpacewarAchievementsOnly() const;

        void ObserveCharacter(Character *character);
        void StopObservingCharacter(Character *character);

    private:
        static constexpr int m_winScoreThreshold{500};

        bool m_isSteamInitialized{};
        bool m_winnerAchievementUnlocked{};
        std::vector<Character *> m_observedCharacters{};
    };
}