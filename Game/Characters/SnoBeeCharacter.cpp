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
            if (auto* enemy = dynamic_cast<BaseEnemy*>(GetOwner()))
            {
                enemy->UpdateFromComponent(deltaTime);
            }
        }
    };
}

namespace
{
    // Utility for random numbers
    int GetRandomInt(int min, int max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd()); // Fixed typo from mt19rng to mt19937
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    bool GetRandomChance(float probability)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd()); // Fixed typo from mt19rng to mt19937
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        return dis(gen) < probability;
    }
}

dae::SnoBeeCharacter::SnoBeeCharacter(ResourceManager &resourceManager, const SnoBeeType* type)
    : BaseEnemy("SnoBee", resourceManager), m_pType(type)
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
}

dae::SnoBeeCharacter::~SnoBeeCharacter() = default;

void dae::SnoBeeCharacter::PerformAction(float dt)
{
    if (health <= 0 && m_currentState != EnemyState::Dead)
    {
        ChangeState(EnemyState::Dead);
    }

    switch (m_currentState)
    {
    case EnemyState::Hatching:
        m_hatchingTimer -= dt;
        if (m_hatchingTimer <= 0.0f)
        {
            ChangeState(EnemyState::Wandering);
        }
        break;

    case EnemyState::Wandering:
    case EnemyState::Chasing:
        ProcessMovement(dt);

        if (!m_isMovingToTarget) // At tile center
        {
            if (m_thinkTimerFrames <= 0)
            {
                Think();
                m_thinkTimerFrames = m_maxThinkFrames;
            }
            else
            {
                m_thinkTimerFrames--;

                // If we didn't think, just keep moving in the same direction if possible
                if (IsTileWalkable(m_currentDirection))
                {
                    m_targetPosition = GetLocalPosition() + glm::vec3(m_currentDirection.x, m_currentDirection.y, 0.0f) * m_blockSize;
                    m_isMovingToTarget = true;
                }
                else
                {
                    // Force a think if blocked
                    Think();
                    m_thinkTimerFrames = m_maxThinkFrames;
                }
            }
        }
        break;

    case EnemyState::BreakingIce:
        m_breakIceTimer -= dt;
        if (m_breakIceTimer <= 0.0f)
        {
            BreakBlockInDirection(m_currentDirection);
            ChangeState(EnemyState::Chasing); // resume chase/wander
            m_targetPosition = GetLocalPosition() + glm::vec3(m_currentDirection.x, m_currentDirection.y, 0.0f) * m_blockSize;
            m_isMovingToTarget = true;
        }
        break;

    case EnemyState::Stunned:
        // Handle stun timer...
        break;

    case EnemyState::Dead:
        // Handle death animation, remove from scene...
        break;
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
    }
    else
    {
        SetLocalPosition(currentPos + glm::normalize(toTarget) * moveDist);
    }
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

    glm::vec2 primaryDir{0.f};
    glm::vec2 secondaryDir{0.f};

    if (std::abs(dx) > std::abs(dy))
    {
        primaryDir = glm::vec2(dx > 0 ? 1.0f : -1.0f, 0.0f);
        secondaryDir = glm::vec2(0.0f, dy > 0 ? 1.0f : -1.0f);
    }
    else
    {
        primaryDir = glm::vec2(0.0f, dy > 0 ? 1.0f : -1.0f);
        secondaryDir = glm::vec2(dx > 0 ? 1.0f : -1.0f, 0.0f);
    }

    // 10-25% randomness
    if (GetRandomChance(0.15f))
    {
        std::vector<glm::vec2> validDirs = GetValidDirections();
        m_currentDirection = GetRandomValidDirection(validDirs);
        return;
    }

    if (IsTileWalkable(primaryDir))
    {
        m_currentDirection = primaryDir;
    }
    else if (IsTileIce(primaryDir) && m_pType && m_pType->isAggressive)
    {
        MaybeBreakIce(primaryDir);
        if (m_currentState == EnemyState::BreakingIce) return;
    }
    else if (IsTileWalkable(secondaryDir))
    {
        m_currentDirection = secondaryDir;
    }
    else if (IsTileIce(secondaryDir) && m_pType && m_pType->isAggressive)
    {
        MaybeBreakIce(secondaryDir);
        if (m_currentState == EnemyState::BreakingIce) return;
    }
    else
    {
        std::vector<glm::vec2> validDirs = GetValidDirections();
        m_currentDirection = GetRandomValidDirection(validDirs, primaryDir);
    }
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
