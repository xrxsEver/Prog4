#include "RemainingLivesDisplayComponent.h"

#include <string>
#include <utility>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"
#include <imgui.h>

dae::RemainingLivesDisplayComponent::RemainingLivesDisplayComponent(GameObject *pOwner, Character *pCharacter)
    : RemainingLivesDisplayComponent(pOwner, pCharacter, "Lives")
{
}

dae::RemainingLivesDisplayComponent::RemainingLivesDisplayComponent(GameObject *pOwner, Character *pCharacter, std::string labelPrefix)
    : Component(pOwner), m_pCharacter(pCharacter), m_labelPrefix(std::move(labelPrefix))
{
    if (m_pCharacter != nullptr)
    {
        m_pCharacter->AddObserver(this);
    }
}

dae::RemainingLivesDisplayComponent::~RemainingLivesDisplayComponent()
{
    if (m_pCharacter != nullptr)
    {
        m_pCharacter->RemoveObserver(this);
    }
}

void dae::RemainingLivesDisplayComponent::Update(float deltaTime)
{
    (void)deltaTime;

    if (m_pTextComponent == nullptr)
    {
        m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
    }

    if (m_pCharacter == nullptr || m_pTextComponent == nullptr)
    {
        return;
    }

    if (m_cachedLives != m_pCharacter->health)
    {
        RefreshText();
    }
}

void dae::RemainingLivesDisplayComponent::OnNotify(const GameEvent event)
{
    if (event == GameEvent::PlayerDied)
    {
        RefreshText();
    }
}

void dae::RemainingLivesDisplayComponent::DrawInspector() const
{
    ImGui::Text("Bound character: %s", m_pCharacter ? m_pCharacter->GetName().c_str() : "missing");
    ImGui::Text("Cached lives: %d", m_cachedLives);
}

void dae::RemainingLivesDisplayComponent::RefreshText()
{
    if (m_pCharacter == nullptr || m_pTextComponent == nullptr)
    {
        return;
    }

    m_cachedLives = m_pCharacter->health;
    m_pTextComponent->SetText(m_labelPrefix + ": " + std::to_string(m_cachedLives));
}
