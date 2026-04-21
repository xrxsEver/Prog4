#include "Achievements.h"

#include <algorithm>

#include "Character.h"

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

namespace
{
    dae::Achievements *g_activeAchievements{};
}

dae::Achievements::Achievements()
{
    g_activeAchievements = this;
#if defined(USE_STEAMWORKS)
    m_isSteamInitialized = SteamAPI_Init();
#endif
}

dae::Achievements::~Achievements()
{
    for (Character *character : m_observedCharacters)
    {
        if (character != nullptr)
        {
            character->RemoveObserver(this);
        }
    }
    m_observedCharacters.clear();

    if (g_activeAchievements == this)
    {
        g_activeAchievements = nullptr;
    }

#if defined(USE_STEAMWORKS)
    if (m_isSteamInitialized)
    {
        SteamAPI_Shutdown();
    }
#endif
}

dae::Achievements *dae::Achievements::GetActiveInstance()
{
    return g_activeAchievements;
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

void dae::Achievements::OnNotify(const GameEvent event)
{
    if (event != GameEvent::PointsGained || m_winnerAchievementUnlocked)
    {
        return;
    }

    for (const Character *character : m_observedCharacters)
    {
        if (character != nullptr && character->score >= m_winScoreThreshold)
        {
            m_winnerAchievementUnlocked = Unlock("ACH_WIN_ONE_GAME");
            return;
        }
    }
}

bool dae::Achievements::IsSteamAvailable() const
{
    return m_isSteamInitialized;
}

std::uint32_t dae::Achievements::GetCurrentAppId() const
{
#if defined(USE_STEAMWORKS)
    if (!m_isSteamInitialized || SteamUtils() == nullptr)
    {
        return 0;
    }

    return SteamUtils()->GetAppID();
#else
    return 0;
#endif
}

bool dae::Achievements::IsSpacewarAppIdActive() const
{
    constexpr std::uint32_t spacewarAppId{480};
    return GetCurrentAppId() == spacewarAppId;
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

bool dae::Achievements::ClearSpacewarAchievementsOnly() const
{
#if defined(USE_STEAMWORKS)
    constexpr AppId_t spacewarAppId{480};

    if (!m_isSteamInitialized || SteamUserStats() == nullptr || SteamUtils() == nullptr)
    {
        return false;
    }

    if (SteamUtils()->GetAppID() != spacewarAppId)
    {
        return false;
    }

    const uint32 achievementCount = SteamUserStats()->GetNumAchievements();
    bool hasClearedAny{};

    for (uint32 index{}; index < achievementCount; ++index)
    {
        const char *achievementName = SteamUserStats()->GetAchievementName(index);
        if (achievementName == nullptr)
        {
            continue;
        }

        hasClearedAny = SteamUserStats()->ClearAchievement(achievementName) || hasClearedAny;
    }

    if (achievementCount == 0)
    {
        return true;
    }

    return hasClearedAny && SteamUserStats()->StoreStats();
#else
    return false;
#endif
}

void dae::Achievements::ObserveCharacter(Character *character)
{
    if (character == nullptr)
    {
        return;
    }

    if (std::find(m_observedCharacters.begin(), m_observedCharacters.end(), character) != m_observedCharacters.end())
    {
        return;
    }

    m_observedCharacters.emplace_back(character);
    character->AddObserver(this);
}

void dae::Achievements::StopObservingCharacter(Character *character)
{
    if (character == nullptr)
    {
        return;
    }

    character->RemoveObserver(this);
    m_observedCharacters.erase(
        std::remove(m_observedCharacters.begin(), m_observedCharacters.end(), character),
        m_observedCharacters.end());
}