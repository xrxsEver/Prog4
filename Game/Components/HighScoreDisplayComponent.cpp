#include "HighScoreDisplayComponent.h"
#include <string>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "HighScores.h"

namespace dae
{
    HighScoreDisplayComponent::HighScoreDisplayComponent(GameObject* pOwner, Character* pCharacter, std::string labelPrefix)
        : Component(pOwner), m_pCharacter(pCharacter), m_labelPrefix(std::move(labelPrefix))
    {
        // Seed from the saved table's best row; the file itself is only written on name entry.
        m_highScore = HighScores::TopScore();
    }

    void HighScoreDisplayComponent::Update(float /*deltaTime*/)
    {
        if (m_pTextComponent == nullptr)
        {
            m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
        }
        if (m_pCharacter == nullptr || m_pTextComponent == nullptr) return;

        // Climb live as the player beats the record, but don't persist -- the table is saved only
        // when a finished run commits its name (see GameController / HighScores::Insert).
        if (m_pCharacter->score > m_highScore)
        {
            m_highScore = m_pCharacter->score;
        }

        if (m_highScore != m_cachedDisplay)
        {
            Refresh();
        }
    }

    std::unique_ptr<Component> HighScoreDisplayComponent::Clone(GameObject* pOwner) const
    {
        return std::make_unique<HighScoreDisplayComponent>(pOwner, m_pCharacter, m_labelPrefix);
    }

    void HighScoreDisplayComponent::Refresh()
    {
        if (m_pTextComponent == nullptr) return;
        m_cachedDisplay = m_highScore;
        m_pTextComponent->SetText(m_labelPrefix + ": " + std::to_string(m_highScore));
    }
}
