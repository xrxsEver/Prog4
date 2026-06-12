#pragma once
#include <string>
#include "Component.h"

namespace dae
{
    class Character;
    class TextComponent;

    // Shows the best score so far, persisted to a file between runs. Polls the player's score
    // and bumps the record (and the file) the moment it is beaten.
    class HighScoreDisplayComponent final : public Component
    {
    public:
        HighScoreDisplayComponent(GameObject* pOwner, Character* pCharacter, std::string labelPrefix = "HI-SCORE");
        ~HighScoreDisplayComponent() override = default;

        void Update(float deltaTime) override;

        const char* GetDebugName() const override { return "High Score Display"; }
        std::unique_ptr<Component> Clone(GameObject* pOwner) const override;

    private:
        void Refresh();

        Character* m_pCharacter{};
        TextComponent* m_pTextComponent{};
        std::string m_labelPrefix;
        int m_highScore{ 0 };
        int m_cachedDisplay{ -1 };
    };
}
