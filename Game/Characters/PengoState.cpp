#include "PengoState.h"
#include "PengoCharacter.h"
#include "GameTime.h"
#include <iostream>

namespace dae
{
    constexpr float ANIMATION_FRAME_DURATION = 0.15f;

    // --- IdleState ---
    void IdleState::OnEnter(PengoCharacter *pPengo)
    {
        PengoDirection dir = pPengo->GetDirection();
        int baseFrame = 0;

        switch (dir)
        {
        case PengoDirection::Down:  baseFrame = 0; break;
        case PengoDirection::Left:  baseFrame = 2; break;
        case PengoDirection::Up:    baseFrame = 4; break;
        case PengoDirection::Right: baseFrame = 6; break;
        }

        pPengo->SetAnimationFrame(baseFrame);
    }

    void IdleState::OnExit(PengoCharacter * /*pPengo*/)
    {
    }

    std::unique_ptr<PengoState> IdleState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    std::unique_ptr<PengoState> IdleState::Update(PengoCharacter *pPengo)
    {
        if (pPengo->HasMoved())
        {
            return std::make_unique<MovingState>();
        }
        return nullptr;
    }

    // --- MovingState ---
    void MovingState::OnEnter(PengoCharacter *pPengo)
    {
        m_animationTimer = 0.0f;
        m_currentFrame = 0;
        UpdateAnimation(pPengo);
    }

    void MovingState::OnExit(PengoCharacter * /*pPengo*/)
    {
    }

    std::unique_ptr<PengoState> MovingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    void MovingState::UpdateAnimation(PengoCharacter *pPengo)
    {
        PengoDirection dir = pPengo->GetDirection();
        int baseFrame = 0;

        switch (dir)
        {
        case PengoDirection::Down:  baseFrame = 0; break;
        case PengoDirection::Left:  baseFrame = 2; break;
        case PengoDirection::Up:    baseFrame = 4; break;
        case PengoDirection::Right: baseFrame = 6; break;
        }

        pPengo->SetAnimationFrame(baseFrame + m_currentFrame);
    }

    std::unique_ptr<PengoState> MovingState::Update(PengoCharacter *pPengo)
    {
        if (!pPengo->HasMoved())
        {
            return std::make_unique<IdleState>();
        }

        m_animationTimer += GameTime::GetInstance().GetDeltaTime();
        if (m_animationTimer >= ANIMATION_FRAME_DURATION)
        {
            m_animationTimer -= ANIMATION_FRAME_DURATION;
            m_currentFrame = (m_currentFrame + 1) % 2;
            UpdateAnimation(pPengo);
        }
        return nullptr;
    }

    // --- PushingState ---
    void PushingState::OnEnter(PengoCharacter * /*pPengo*/)
    {
    }

    void PushingState::OnExit(PengoCharacter * /*pPengo*/)
    {
    }

    std::unique_ptr<PengoState> PushingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    std::unique_ptr<PengoState> PushingState::Update(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    // --- DyingState ---
    void DyingState::OnEnter(PengoCharacter * /*pPengo*/)
    {
    }

    void DyingState::OnExit(PengoCharacter * /*pPengo*/)
    {
    }

    std::unique_ptr<PengoState> DyingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    std::unique_ptr<PengoState> DyingState::Update(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }
}