#pragma once

namespace dae
{
    class GameObject;

    class State
    {
    public:
        virtual ~State() = default;

        virtual void OnEnter() {}
        virtual void OnExit() {}
        virtual void Update(GameObject &actor, float dt) = 0;
    };
}