#include "Achievements.h"

#if defined(USE_STEAMWORKS)
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
#include <steam_api.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif

dae::Achievements::Achievements()
{
#if defined(USE_STEAMWORKS)
    m_isSteamInitialized = SteamAPI_Init();
#endif
}

dae::Achievements::~Achievements()
{
#if defined(USE_STEAMWORKS)
    if (m_isSteamInitialized)
    {
        SteamAPI_Shutdown();
    }
#endif
}

void dae::Achievements::Update()
{
#if defined(USE_STEAMWORKS)
    if (m_isSteamInitialized)
    {
        SteamAPI_RunCallbacks();
    }
#endif
}

bool dae::Achievements::IsSteamAvailable() const
{
    return m_isSteamInitialized;
}

bool dae::Achievements::Unlock(std::string_view achievementId) const
{
#if defined(USE_STEAMWORKS)
    if (!m_isSteamInitialized || achievementId.empty() || SteamUserStats() == nullptr)
    {
        return false;
    }

    const bool wasSet = SteamUserStats()->SetAchievement(achievementId.data());
    if (!wasSet)
    {
        return false;
    }

    return SteamUserStats()->StoreStats();
#else
    (void)achievementId;
    return false;
#endif
}