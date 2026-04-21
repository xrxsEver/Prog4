#include "ScoreDisplayComponent.h"

#include <string>
#include <utility>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"
#include <imgui.h>

namespace
{
    int CalculateTotalScore(const std::vector<dae::Character *> &characters)
    {
        int totalScore{};
        for (const auto *character : characters)
        {
            if (character != nullptr)
            {
                totalScore += character->score;
            }
        }
        return totalScore;
    }
}

dae::ScoreDisplayComponent::ScoreDisplayComponent(GameObject *pOwner, std::vector<Character *> observedCharacters, std::string labelPrefix)
    : Component(pOwner), m_observedCharacters(std::move(observedCharacters)), m_labelPrefix(std::move(labelPrefix))
{
    for (auto *character : m_observedCharacters)
    {
        if (character != nullptr)
        {
            character->AddObserver(this);
        }
    }
}

dae::ScoreDisplayComponent::~ScoreDisplayComponent()
{
    for (auto *character : m_observedCharacters)
    {
        if (character != nullptr)
        {
            character->RemoveObserver(this);
        }
    }
}

void dae::ScoreDisplayComponent::Update(float deltaTime)
{
    (void)deltaTime;

    if (m_pTextComponent == nullptr)
    {
        m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
    }

    if (m_pTextComponent == nullptr)
    {
        return;
    }

    const int currentScore = CalculateTotalScore(m_observedCharacters);
    if (currentScore != m_cachedScore)
    {
        RefreshText();
    }
}

void dae::ScoreDisplayComponent::OnNotify(const GameEvent event)
{
    if (event == GameEvent::PointsGained)
    {
        RefreshText();
    }
}

void dae::ScoreDisplayComponent::DrawInspector() const
{
    ImGui::Text("Observed characters: %zu", m_observedCharacters.size());
    ImGui::Text("Cached score: %d", m_cachedScore);
}

void dae::ScoreDisplayComponent::RefreshText()
{
    if (m_pTextComponent == nullptr)
    {
        return;
    }

    m_cachedScore = CalculateTotalScore(m_observedCharacters);
    m_pTextComponent->SetText(m_labelPrefix + ": " + std::to_string(m_cachedScore));
}
