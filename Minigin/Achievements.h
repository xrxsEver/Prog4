#pragma once

#include <string_view>

namespace dae
{
    class Achievements final
    {
    public:
        Achievements();
        ~Achievements();

        Achievements(const Achievements &other) = delete;
        Achievements(Achievements &&other) = delete;
        Achievements &operator=(const Achievements &other) = delete;
        Achievements &operator=(Achievements &&other) = delete;

        void Update();
        [[nodiscard]] bool IsSteamAvailable() const;
        [[nodiscard]] bool Unlock(std::string_view achievementId) const;

    private:
        bool m_isSteamInitialized{};
    };
}