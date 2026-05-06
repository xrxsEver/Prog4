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
        std::cout << "Pengo enters IdleState\n";
        pPengo->SetAnimationFrame(0);
    }

    void IdleState::OnExit(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo exits IdleState\n";
    }

    PengoState *IdleState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    PengoState *IdleState::Update(PengoCharacter *pPengo)
    {
        if (pPengo->HasMoved())
        {
            return new MovingState();
        }
        return nullptr;
    }

    // --- MovingState ---
    void MovingState::OnEnter(PengoCharacter *pPengo)
    {
        std::cout << "Pengo enters MovingState\n";
        m_animationTimer = 0.0f;

        PengoDirection dir = pPengo->GetDirection();
        int baseFrame = 0;

        switch (dir)
        {
        case PengoDirection::Down:
            baseFrame = 0;
            break;
        case PengoDirection::Left:
            baseFrame = 2;
            break;
        case PengoDirection::Up:
            baseFrame = 4;
            break;
        case PengoDirection::Right:
            baseFrame = 6;
            break;
        }

        m_currentFrame = baseFrame;
        pPengo->SetAnimationFrame(m_currentFrame);
    }

    void MovingState::OnExit(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo exits MovingState\n";
    }

    PengoState *MovingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    PengoState *MovingState::Update(PengoCharacter *pPengo)
    {
        if (!pPengo->HasMoved())
        {
            return new IdleState();
        }

        m_animationTimer += GameTime::GetInstance().GetDeltaTime();
        if (m_animationTimer >= ANIMATION_FRAME_DURATION)
        {
            m_animationTimer -= ANIMATION_FRAME_DURATION;

            PengoDirection dir = pPengo->GetDirection();
            int baseFrame = 0;

            switch (dir)
            {
            case PengoDirection::Down:
                baseFrame = 0;
                break;
            case PengoDirection::Left:
                baseFrame = 2;
                break;
            case PengoDirection::Up:
                baseFrame = 4;
                break;
            case PengoDirection::Right:
                baseFrame = 6;
                break;
            }

            if (m_currentFrame == baseFrame)
            {
                m_currentFrame = baseFrame + 1;
            }
            else
            {
                m_currentFrame = baseFrame;
            }
            pPengo->SetAnimationFrame(m_currentFrame);
        }
        return nullptr;
    }

    // --- PushingState ---
    void PushingState::OnEnter(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo enters PushingState\n";
    }

    void PushingState::OnExit(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo exits PushingState\n";
    }

    PengoState *PushingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    PengoState *PushingState::Update(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    // --- DyingState ---
    void DyingState::OnEnter(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo enters DyingState\n";
    }

    void DyingState::OnExit(PengoCharacter * /*pPengo*/)
    {
        std::cout << "Pengo exits DyingState\n";
    }

    PengoState *DyingState::HandleInput(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }

    PengoState *DyingState::Update(PengoCharacter * /*pPengo*/)
    {
        return nullptr;
    }
}