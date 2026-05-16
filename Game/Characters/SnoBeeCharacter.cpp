#include "SnoBeeCharacter.h"
#include "GameObject.h"

#include <cmath>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <random>

#include "AnalogStickMoveComponent.h"
#include "AddScoreCommand.h"
#include "InputManager.h"
#include "LoseLifeCommand.h"
#include "MoveCommand.h"
#include "Component.h"
#include "GridObjectComponent.h"

namespace dae
{
    class BaseEnemyUpdateComponent final : public Component
    {
    public:
        explicit BaseEnemyUpdateComponent(GameObject* owner) : Component(owner) {}

        std::unique_ptr<Component> Clone(GameObject* pOwner) const override
        {
            return std::make_unique<BaseEnemyUpdateComponent>(pOwner);
        }

        void Update(float deltaTime) override
        {
            auto* enemy = dynamic_cast<BaseEnemy*>(GetOwner());
            if (!enemy) return;
            enemy->UpdateFromComponent(deltaTime);
        }
    };
}

namespace
{
    // Utility for random numbers
    int GetRandomInt(int min, int max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    bool GetRandomChance(float probability)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        return dis(gen) < probability;
    }

    int g_SnoBeeCounter = 0;
}

dae::SnoBeeCharacter::SnoBeeCharacter(ResourceManager &resourceManager, const SnoBeeType* type)
    : BaseEnemy("SnoBee " + std::to_string(++g_SnoBeeCounter), resourceManager), m_pType(type)
{
    InitializeSprite(8 * 16.0f, 9 * 16.0f);
    ChangeState(EnemyState::Hatching);

    m_targetPosition = GetLocalPosition();
    m_thinkTimerFrames = m_maxThinkFrames;

    if (m_pType)
    {
        score = m_pType->scoreValue;
        SetSpriteSourceRect(8 * 16.0f, static_cast<float>(9 + m_pType->spriteSheetRowOffset) * 16.0f, 16.0f, 16.0f);
    }

    AddComponent<BaseEnemyUpdateComponent>();
    AddComponent<GridObjectComponent>();
}

dae::SnoBeeCharacter::~SnoBeeCharacter() = default;

void dae::SnoBeeCharacter::PerformAction(float dt)
{
    if (health <= 0 && m_currentState != EnemyState::Dead)
    {
        ChangeState(EnemyState::Dead);
    }

    if (m_currentState == EnemyState::Dead)
    {
        return;
    }

    if (m_currentState == EnemyState::Hatching)
    {
        m_hatchingTimer -= dt;
        if (m_hatchingTimer <= 0.0f)
        {
            ChangeState(EnemyState::Wandering);
        }
        return;
    }

    if (m_currentState == EnemyState::Wandering || m_currentState == EnemyState::Chasing)
    {
        ProcessMovement(dt);

        if (m_isMovingToTarget) return;

        if (m_thinkTimerFrames <= 0)
        {
            Think();
            m_thinkTimerFrames = m_maxThinkFrames;
            return;
        }

        m_thinkTimerFrames--;

        if (IsTileWalkable(m_currentDirection))
        {
            m_targetPosition = GetLocalPosition() + glm::vec3(m_currentDirection.x, m_currentDirection.y, 0.0f) * m_blockSize;
            m_isMovingToTarget = true;
            return;
        }

        Think();
        m_thinkTimerFrames = m_maxThinkFrames;
        return;
    }

    if (m_currentState == EnemyState::BreakingIce)
    {
        m_breakIceTimer -= dt;
        if (m_breakIceTimer <= 0.0f)
        {
            BreakBlockInDirection(m_currentDirection);
            ChangeState(EnemyState::Chasing);
            m_targetPosition = GetLocalPosition() + glm::vec3(m_currentDirection.x, m_currentDirection.y, 0.0f) * m_blockSize;
            m_isMovingToTarget = true;
        }
        return;
    }
}

void dae::SnoBeeCharacter::ProcessMovement(float dt)
{
    if (!m_isMovingToTarget) return;

    float speed = m_pType ? m_pType->speed : 100.0f;
    glm::vec3 currentPos = GetLocalPosition();
    glm::vec3 toTarget = m_targetPosition - currentPos;
    float distance = glm::length(toTarget);
    float moveDist = speed * dt;

    if (moveDist >= distance)
    {
        SetLocalPosition(m_targetPosition);
        m_isMovingToTarget = false;
        return; // Early return
    }

    SetLocalPosition(currentPos + glm::normalize(toTarget) * moveDist);
}

void dae::SnoBeeCharacter::Think()
{
    // Determine whether to chase or wander
    // Arcade style: simple line of sight or global tracking depending on AI Tier
    bool canSeePlayer = true; // In arcade, enemies often always know player pos, just track differently

    if (canSeePlayer)
    {
        ChasePlayer();
    }
    else
    {
        Wander();
    }

    if (m_currentState != EnemyState::BreakingIce && IsTileWalkable(m_currentDirection))
    {
        m_targetPosition = GetLocalPosition() + glm::vec3(m_currentDirection.x, m_currentDirection.y, 0.0f) * m_blockSize;
        m_isMovingToTarget = true;
    }
}

void dae::SnoBeeCharacter::ChasePlayer()
{
    ChangeState(EnemyState::Chasing);

    glm::vec3 playerPos = GetPlayerPosition();
    glm::vec3 myPos = GetLocalPosition();

    float dx = playerPos.x - myPos.x;
    float dy = playerPos.y - myPos.y;

    // 10-25% randomness
    if (GetRandomChance(0.15f))
    {
        m_currentDirection = GetRandomValidDirection(GetValidDirections());
        return;
    }

    glm::vec2 primaryDir = (std::abs(dx) > std::abs(dy))
        ? glm::vec2(dx > 0 ? 1.0f : -1.0f, 0.0f)
        : glm::vec2(0.0f, dy > 0 ? 1.0f : -1.0f);

    glm::vec2 secondaryDir = (std::abs(dx) > std::abs(dy))
        ? glm::vec2(0.0f, dy > 0 ? 1.0f : -1.0f)
        : glm::vec2(dx > 0 ? 1.0f : -1.0f, 0.0f);

    if (IsTileWalkable(primaryDir))
    {
        m_currentDirection = primaryDir;
        return;
    }

    if (IsTileIce(primaryDir) && m_pType && m_pType->isAggressive)
    {
        MaybeBreakIce(primaryDir);
        if (m_currentState == EnemyState::BreakingIce) return;
    }

    if (IsTileWalkable(secondaryDir))
    {
        m_currentDirection = secondaryDir;
        return;
    }

    if (IsTileIce(secondaryDir) && m_pType && m_pType->isAggressive)
    {
        MaybeBreakIce(secondaryDir);
        if (m_currentState == EnemyState::BreakingIce) return;
    }

    m_currentDirection = GetRandomValidDirection(GetValidDirections(), primaryDir);
}

void dae::SnoBeeCharacter::Wander()
{
    ChangeState(EnemyState::Wandering);
    std::vector<glm::vec2> validDirs = GetValidDirections();
    m_currentDirection = GetRandomValidDirection(validDirs);
}

void dae::SnoBeeCharacter::MaybeBreakIce(const glm::vec2& blockedDir)
{
    float aggression = m_pType && m_pType->isAggressive ? 0.5f : 0.1f;
    if (GetRandomChance(aggression))
    {
        ChangeState(EnemyState::BreakingIce);
        m_currentDirection = blockedDir;
        m_breakIceTimer = 1.0f; // 1 second to break ice
    }
}

std::vector<glm::vec2> dae::SnoBeeCharacter::GetValidDirections() const
{
    std::vector<glm::vec2> valid;
    glm::vec2 dirs[] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    for (const auto& d : dirs)
    {
        if (IsTileWalkable(d))
        {
            valid.push_back(d);
        }
    }
    return valid;
}

glm::vec2 dae::SnoBeeCharacter::GetRandomValidDirection(const std::vector<glm::vec2>& validDirs, const glm::vec2& preferredDir) const
{
    if (validDirs.empty()) return -m_currentDirection; // Trap reverse

    std::vector<glm::vec2> filtered;
    for (const auto& d : validDirs)
    {
        // Avoid reversing unless it's the only option
        if (d != -m_currentDirection || validDirs.size() == 1)
        {
            filtered.push_back(d);
        }
    }

    if (filtered.empty()) return -m_currentDirection;

    // Small bias toward preferred if possible, otherwise purely random
    if (preferredDir != glm::vec2{0.f} && std::find(filtered.begin(), filtered.end(), preferredDir) != filtered.end() && GetRandomChance(0.5f))
    {
        return preferredDir;
    }

    int idx = GetRandomInt(0, static_cast<int>(filtered.size() - 1));
    return filtered[idx];
}

void dae::SnoBeeCharacter::ChangeState(const EnemyState nextState)
{
    m_currentState = nextState;
    if (m_currentState == EnemyState::Dead)
    {
        if (auto pGridComp = GetComponent<GridObjectComponent>())
        {
            pGridComp->Disable();
        }
    }
}

void dae::SnoBeeCharacter::BindGamepadControls(InputManager &inputManager, const std::uint32_t gamepadIndex)
{
    float currentSpeed = m_pType ? m_pType->speed : 400.0f;
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadUp, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, -1.0f}, currentSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadDown, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{0.0f, 1.0f}, currentSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadLeft, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{-1.0f, 0.0f}, currentSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::DPadRight, KeyState::Pressed, std::make_unique<MoveCommand>(*this, glm::vec2{1.0f, 0.0f}, currentSpeed));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::X, KeyState::Down, std::make_unique<LoseLifeCommand>(*this));

    int currentScore = m_pType ? m_pType->scoreValue : 100;
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::A, KeyState::Down, std::make_unique<AddScoreCommand>(*this, currentScore));
    inputManager.BindGamepadCommand(gamepadIndex, Gamepad::Button::B, KeyState::Down, std::make_unique<AddScoreCommand>(*this, currentScore * 10));

    m_pMoveComponent = AddComponent<AnalogStickMoveComponent>(inputManager, gamepadIndex, currentSpeed);
}

std::unique_ptr<dae::SnoBeeCharacter> dae::SnoBeeCharacter::Clone() const
{
    auto clone = std::make_unique<SnoBeeCharacter>(m_resourceManager, m_pType);
    return clone;
}

std::unique_ptr<dae::SnoBeeCharacter> dae::SnoBeeCharacter::Spawn(const SnoBeeCharacter& prototype, const SnoBeeType* type)
{
    auto cloned = prototype.Clone();
    if (type)
    {
        cloned->m_pType = type;
        cloned->score = type->scoreValue;
        cloned->SetSpriteSourceRect(8 * 16.0f, static_cast<float>(9 + type->spriteSheetRowOffset) * 16.0f, 16.0f, 16.0f);

        if (cloned->m_pMoveComponent)
        {
            cloned->m_pMoveComponent->SetSpeed(type->speed);
        }
    }
    return cloned;
}
