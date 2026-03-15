#include "Subject.h"

#include <algorithm>

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
    m_Observers.erase(
        std::remove(m_Observers.begin(), m_Observers.end(), observer),
        m_Observers.end());
}

void dae::Subject::NotifyObservers(const GameEvent event)
{
    for (Observer *observer : m_Observers)
    {
        if (observer != nullptr)
        {
            observer->OnNotify(event);
        }
    }
}