#include "HighScoreDisplayComponent.h"
#include <fstream>
#include <string>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"

namespace
{
    // Kept next to the executable (not in Data/, which the build recopies each time)
    constexpr const char* HIGHSCORE_FILE = "highscore.txt";

    int LoadHighScore()
    {
        std::ifstream file(HIGHSCORE_FILE);
        int value = 0;
        if (file >> value) return value;
        return 0;
    }

    void SaveHighScore(int value)
    {
        std::ofstream file(HIGHSCORE_FILE, std::ios::trunc);
        if (file) file << value;
    }
}

namespace dae
{
    HighScoreDisplayComponent::HighScoreDisplayComponent(GameObject* pOwner, Character* pCharacter, std::string labelPrefix)
        : Component(pOwner), m_pCharacter(pCharacter), m_labelPrefix(std::move(labelPrefix))
    {
        m_highScore = LoadHighScore();
    }

    void HighScoreDisplayComponent::Update(float /*deltaTime*/)
    {
        if (m_pTextComponent == nullptr)
        {
            m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
        }
        if (m_pCharacter == nullptr || m_pTextComponent == nullptr) return;

        if (m_pCharacter->score > m_highScore)
        {
            m_highScore = m_pCharacter->score;
            SaveHighScore(m_highScore);
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
