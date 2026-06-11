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
    void PushingState::OnEnter(PengoCharacter *pPengo)
    {
        m_timer = 0.0f;
        
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
        pPengo->SetSpriteData(1, 0, false); // Row 1 is Pushing
    }

    void PushingState::OnExit(PengoCharacter *pPengo)
    {
        pPengo->SetSpriteData(0, 0, false); // Back to Row 0
    }

    std::unique_ptr<PengoState> PushingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    std::unique_ptr<PengoState> PushingState::Update(PengoCharacter * /*pPengo*/)
    {
        m_timer += GameTime::GetInstance().GetDeltaTime();
        if (m_timer >= PUSH_DURATION)
        {
            return std::make_unique<IdleState>();
        }
        return nullptr;
    }

    // --- DyingState ---
    void DyingState::OnEnter(PengoCharacter *pPengo)
    {
        m_animationTimer = 0.0f;
        m_currentFrame = 0;

        // Death frames live one row below the walk row, same columns
        pPengo->SetSpriteData(1, 0, false);
        UpdateAnimation(pPengo);
    }

    void DyingState::OnExit(PengoCharacter *pPengo)
    {
        pPengo->SetSpriteData(0, 0, false); // back to the walk row
    }

    std::unique_ptr<PengoState> DyingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    void DyingState::UpdateAnimation(PengoCharacter *pPengo)
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

    std::unique_ptr<PengoState> DyingState::Update(PengoCharacter *pPengo)
    {
        // Keep looping the death animation; the level coordinator pulls us out via Respawn()
        m_animationTimer += GameTime::GetInstance().GetDeltaTime();
        if (m_animationTimer >= ANIMATION_FRAME_DURATION)
        {
            m_animationTimer -= ANIMATION_FRAME_DURATION;
            m_currentFrame = (m_currentFrame + 1) % 2;
            UpdateAnimation(pPengo);
        }
        return nullptr;
    }
}