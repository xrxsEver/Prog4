#include "RemainingLivesDisplayComponent.h"
#include <format>
#include <iostream>
#include <stdexcept>
#include <imgui.h>

#include "Character.h"
#include "GameObject.h"
#include "TextComponent.h"

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

    if (m_pCharacter->health != m_cachedLives)
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

std::unique_ptr<dae::Component> dae::RemainingLivesDisplayComponent::Clone(GameObject* pOwner) const
{
    return std::make_unique<RemainingLivesDisplayComponent>(pOwner, m_pCharacter, m_labelPrefix);
}

void dae::RemainingLivesDisplayComponent::DrawInspector() const
{
    ImGui::Text("Observing: %s", m_pCharacter ? m_pCharacter->GetName().c_str() : "none");
    ImGui::Text("Label prefix: %s", m_labelPrefix.c_str());
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
