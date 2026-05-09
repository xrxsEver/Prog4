#include "ScoreDisplayComponent.h"

#include <numeric>
#include <string>
#include <imgui.h>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"

dae::ScoreDisplayComponent::ScoreDisplayComponent(GameObject *pOwner, std::vector<Character *> observedCharacters, std::string labelPrefix)
    : Component(pOwner), m_observedCharacters(std::move(observedCharacters)), m_labelPrefix(std::move(labelPrefix))
{
    for (const auto character : m_observedCharacters)
    {
        if (character != nullptr)
        {
            character->AddObserver(this);
        }
    }
}

dae::ScoreDisplayComponent::~ScoreDisplayComponent()
{
    for (const auto character : m_observedCharacters)
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

    const int currentScore = std::accumulate(m_observedCharacters.begin(), m_observedCharacters.end(), 0, [](const int sum, const Character *character)
                                             { return sum + (character != nullptr ? character->score : 0); });

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

std::unique_ptr<dae::Component> dae::ScoreDisplayComponent::Clone(GameObject* pOwner) const
{
    return std::make_unique<ScoreDisplayComponent>(pOwner, m_observedCharacters, m_labelPrefix);
}

void dae::ScoreDisplayComponent::DrawInspector() const
{
    ImGui::Text("Label prefix: %s", m_labelPrefix.c_str());
    ImGui::Text("Cached score: %d", m_cachedScore);
    ImGui::Text("Observing %zu characters", m_observedCharacters.size());
}

void dae::ScoreDisplayComponent::RefreshText()
{
    if (m_pTextComponent == nullptr)
    {
        return;
    }

    m_cachedScore = std::accumulate(m_observedCharacters.begin(), m_observedCharacters.end(), 0, [](const int sum, const Character *character)
                                    { return sum + (character != nullptr ? character->score : 0); });

    m_pTextComponent->SetText(m_labelPrefix + ": " + std::to_string(m_cachedScore));
}
