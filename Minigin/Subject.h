#ifndef MINIGIN_SUBJECT_H
#define MINIGIN_SUBJECT_H

#pragma once
#include <vector>
#include "Observer.h"

namespace dae {
    class Subject {
    public:
        void AddObserver(Observer* observer);
        void RemoveObserver(Observer* observer);
    protected:
        void NotifyObservers(GameEvent event);
    private:
        std::vector<Observer*> m_Observers;
    };
}

#endif //MINIGIN_SUBJECT_H