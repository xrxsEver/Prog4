#ifndef MINIGIN_EVENTMANAGER_H
#define MINIGIN_EVENTMANAGER_H

#pragma once
#include "Singleton.h"
#include "Event.h"
#include <functional>
#include <map>

namespace dae {
    class EventManager final : public Singleton<EventManager> {
    public:
        using EventHandler = std::function<void(const Event&)>;

        void Subscribe(EventId id, EventHandler handler);
        void Broadcast(const Event& event);              //SendEvent
    };
}
#endif //MINIGIN_EVENTMANAGER_H