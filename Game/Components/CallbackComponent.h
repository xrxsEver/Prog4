#pragma once
#include <functional>
#include <utility>
#include "Component.h"

namespace dae
{
    // Runs a std::function every Update. Used to "pump" the GameController each frame from
    // whichever scene is active, so deferred menu/mode transitions run at a safe point
    // (outside the input-dispatch loop, where rebinding would otherwise be unsafe).
    class CallbackComponent final : public Component
    {
    public:
        CallbackComponent(GameObject* pOwner, std::function<void()> onUpdate)
            : Component(pOwner), m_onUpdate(std::move(onUpdate)) {}

        void Update(float /*deltaTime*/) override
        {
            if (m_onUpdate)
            {
                m_onUpdate();
            }
        }

        const char* GetDebugName() const override { return "Callback"; }

        std::unique_ptr<Component> Clone(GameObject* pOwner) const override
        {
            return std::make_unique<CallbackComponent>(pOwner, m_onUpdate);
        }

    private:
        std::function<void()> m_onUpdate;
    };
}
