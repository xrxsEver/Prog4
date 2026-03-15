#ifndef MINIGIN_OBSERVER_H
#define MINIGIN_OBSERVER_H

namespace dae
{
    enum class GameEvent
    {
        PlayerDied,
        ActorFell,
        PointsGained
    };

    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void OnNotify(GameEvent event) = 0;
    };
}

#endif // MINIGIN_OBSERVER_H