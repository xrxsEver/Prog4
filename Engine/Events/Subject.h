#ifndef MINIGIN_SUBJECT_H
#define MINIGIN_SUBJECT_H

#pragma once
#include <cstdint>
#include <vector>
#include "Observer.h"

namespace dae
{
    class Subject
    {
    public:
        struct NotificationRecord
        {
            std::uint64_t sequence{};
            std::uintptr_t subjectAddress{};
            GameEvent event{GameEvent::PlayerDied};
            std::size_t observerCount{};
        };

        Subject();
        virtual ~Subject();

        void AddObserver(Observer *observer);
        void RemoveObserver(Observer *observer);
        [[nodiscard]] std::size_t GetObserverCount() const;

        static const std::vector<Subject *> &GetLiveSubjects();
        static const std::vector<NotificationRecord> &GetRecentNotifications();

    protected:
        void NotifyObservers(GameEvent event);
    private:
        std::vector<Observer*> m_Observers;
    };
}

#endif //MINIGIN_SUBJECT_H