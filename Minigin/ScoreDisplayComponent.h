#pragma once

#include <string>
#include <vector>

#include "Component.h"
#include "Observer.h"

namespace dae
{
    class Character;
    class TextComponent;

    class ScoreDisplayComponent final : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject *pOwner, std::vector<Character *> observedCharacters, std::string labelPrefix = "Score");
        ~ScoreDisplayComponent() override;

        void Update() override;
        void OnNotify(GameEvent event) override;
        const char *GetDebugName() const override { return "ScoreDisplay"; }
        void DrawInspector() const override;

    private:
        void RefreshText();

        std::vector<Character *> m_observedCharacters;
        TextComponent *m_pTextComponent{};
        std::string m_labelPrefix;
        int m_cachedScore{-1};
    };
}
