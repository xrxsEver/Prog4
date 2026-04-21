#pragma once

#include <string>

#include "Component.h"
#include "Observer.h"

namespace dae
{
    class Character;
    class TextComponent;

    class RemainingLivesDisplayComponent final : public Component, public Observer
    {
    public:
        RemainingLivesDisplayComponent(GameObject *pOwner, Character *pCharacter);
        RemainingLivesDisplayComponent(GameObject *pOwner, Character *pCharacter, std::string labelPrefix);
        ~RemainingLivesDisplayComponent() override;

        void Update(float deltaTime) override;
        void OnNotify(GameEvent event) override;
        const char *GetDebugName() const override { return "RemainingLivesDisplay"; }
        void DrawInspector() const override;

    private:
        void RefreshText();

        Character *m_pCharacter{};
        TextComponent *m_pTextComponent{};
        std::string m_labelPrefix;
        int m_cachedLives{-1};
    };
}
