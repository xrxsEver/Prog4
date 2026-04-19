#include "Subject.h"

#include <algorithm>

namespace
{
    constexpr std::size_t g_maxStoredNotifications{64};

    std::vector<dae::Subject *> g_liveSubjects{};
    std::vector<dae::Subject::NotificationRecord> g_recentNotifications{};
    std::uint64_t g_notificationSequence{};
}

dae::Subject::Subject()
{
    g_liveSubjects.emplace_back(this);
}

dae::Subject::~Subject()
{
    m_Observers.clear();
    g_liveSubjects.erase(
        std::remove(g_liveSubjects.begin(), g_liveSubjects.end(), this),
        g_liveSubjects.end());
}

void dae::Subject::AddObserver(Observer *observer)
{
    if (observer == nullptr)
    {
        return;
    }

    if (std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end())
    {
        m_Observers.emplace_back(observer);
    }
}

void dae::Subject::RemoveObserver(Observer *observer)
{
    if (observer == nullptr)
    {
        return;
    }

    m_Observers.erase(
        std::remove(m_Observers.begin(), m_Observers.end(), observer),
        m_Observers.end());
}

void dae::Subject::NotifyObservers(const GameEvent event)
{
    g_recentNotifications.emplace_back(NotificationRecord{
        ++g_notificationSequence,
        reinterpret_cast<std::uintptr_t>(this),
        event,
        m_Observers.size()});

    if (g_recentNotifications.size() > g_maxStoredNotifications)
    {
        g_recentNotifications.erase(g_recentNotifications.begin());
    }

    for (Observer *observer : m_Observers)
    {
        if (observer != nullptr)
        {
            observer->OnNotify(event);
        }
    }
}

std::size_t dae::Subject::GetObserverCount() const
{
    return m_Observers.size();
}

const std::vector<dae::Subject *> &dae::Subject::GetLiveSubjects()
{
    return g_liveSubjects;
}

const std::vector<dae::Subject::NotificationRecord> &dae::Subject::GetRecentNotifications()
{
    return g_recentNotifications;
}