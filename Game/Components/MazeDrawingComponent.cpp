#include "MazeDrawingComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "MazeGenerator.h"
#include "Scene.h"
#include "SnoBeeCharacter.h"
#include "PengoCharacter.h"
#include "TypeRegistry.h"
#include "CollisionGrid.h"
#include <algorithm>
#include <array>
#include <random>

namespace dae
{
    bool MazeDrawingComponent::g_ShowFullMaze = false;

    MazeDrawingComponent::MazeDrawingComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager, const std::string& levelFile, std::function<void(glm::vec2)> onFinished)
        : Component(owner)
        , m_scene(scene)
        , m_resourceManager(resourceManager)
        , m_onFinished(onFinished)
        , m_levelFile(levelFile)
    {
        m_backgroundTexture = m_resourceManager.LoadTexture("Playfield.png");
        m_iceBlockTexture = m_resourceManager.LoadTexture("iceblock.png");
        m_miscTexture = m_resourceManager.LoadTexture("misc.png");
        m_pengoTexture = m_resourceManager.LoadTexture("pengo.png");

        m_pIceBlockPool = std::make_unique<IceBlockPool>(scene, resourceManager);

        MazeGenerator generator;
        const auto result = generator.LoadFromFile(m_resourceManager.GetDataPath() + levelFile);

        m_rows = result.rows;
        m_cols = result.cols;

        m_pengoSpawnPos = GetScreenPos(result.pengoSpawn.first, result.pengoSpawn.second);

        m_offsetX = 0.0f;
        m_offsetY = 0.0f;

        ServiceLocator::get_collision_grid().SetRenderOffset(m_offsetX + m_blockSize, m_offsetY + m_blockSize);

        // Initialize grid
        m_blocks.reserve(m_rows * m_cols);
        for (int r = 0; r < m_rows; ++r)
        {
            for (int c = 0; c < m_cols; ++c)
            {
                IceBlock* pBlock = nullptr;
                // Only acquire ice blocks for non-WALL tiles
                if (result.grid[r][c] != TileType::WALL)
                {
                    pBlock = m_pIceBlockPool->Acquire();
                    if (pBlock)
                    {
                        const glm::vec2 screenPos = GetScreenPos(r, c);
                        pBlock->SetPosition(screenPos.x, screenPos.y);
                    }
                }
                m_blocks.push_back(MazeBlock{ r, c, false, result.grid[r][c], false, 0, 0.0f, pBlock });
            }
        }

        // The removal order follows the carving sequence
        for (const auto& pos : result.carvingSequence)
        {
            // Find index of block at pos
            const int index = pos.first * m_cols + pos.second;
            m_removalOrder.push_back(index);
        }
    }

    void MazeDrawingComponent::Update(float deltaTime)
    {
        // Once the intro is over the maze runs as a little state machine
        switch (m_phase)
        {
        case LevelPhase::Playing:   UpdatePlaying(deltaTime);   return;
        case LevelPhase::DeathWipe: UpdateDeathWipe(deltaTime); return;
        case LevelPhase::DeathHold: UpdateDeathHold(deltaTime); return;
        default: break; // Intro keeps using the original draw / hatch logic below
        }

        m_pIceBlockPool->Update(deltaTime);

        if (m_isFinished)
        {
            AdvanceSpawnAnimation(deltaTime);
            return;
        }

        m_timer += deltaTime;
        if (m_timer >= m_blockRemoveInterval)
        {
            m_timer = 0.0f;
            if (m_removedStep < static_cast<int>(m_removalOrder.size()))
            {
                const int index = m_removalOrder[m_removedStep];
                auto& b = m_blocks[index];
                b.removed = true;
                
                if (b.pPooledBlock)
                {
                    m_pIceBlockPool->Release(b.pPooledBlock);
                    b.pPooledBlock = nullptr;
                }
                
                m_removedStep++;
            }
            else
            {
                m_isFinished = true;
                // Finished! Stop drawing sound and play start sound
                ServiceLocator::get_sound_system().stop_music();
                ServiceLocator::get_sound_system().play_music("Sounds/Start.mp3", 0.5f, false);
                
                // Pick 3 random remaining ice blocks
                std::vector<int> iceBlockIndices;
                for (int i = 0; i < static_cast<int>(m_blocks.size()); ++i)
                {
                    if (!m_blocks[i].removed && m_blocks[i].type == TileType::ICE)
                    {
                        iceBlockIndices.push_back(i);
                    }
                }

                if (iceBlockIndices.size() >= 3)
                {
                    std::random_device rd;
                    std::mt19937 g(rd());
                    std::shuffle(iceBlockIndices.begin(), iceBlockIndices.end(), g);
                    
                    for (int i = 0; i < 3; ++i)
                    {
                        const int index = iceBlockIndices[i];
                        m_spawnBlockIndices.push_back(index);
                        auto& b = m_blocks[index];
                        b.isSpawning = true;
                        if (b.pPooledBlock)
                        {
                            m_pIceBlockPool->Release(b.pPooledBlock);
                            b.pPooledBlock = nullptr;
                        }
                    }
                    m_hatchingReserve = false; // this is the intro wave, not a reserve hatch
                    m_spawnStep = SpawnStep::IceBreaking;
                    m_spawnAnimationFrame = 0;
                    m_spawnAnimationTimer = 0.0f;
                }
                else
                {
                    if (m_onFinished) m_onFinished(m_pengoSpawnPos);
                    m_phase = LevelPhase::Playing; // no hatch to play, straight to gameplay
                    SetupDiamonds();
                    SetupReserveEggs();
                }
            }
        }
    }

    void MazeDrawingComponent::UpdatePlaying(float deltaTime)
    {
        m_pIceBlockPool->Update(deltaTime);

        // Pengo dying kicks off the life-lost sequence
        if (m_pPengo && m_pPengo->IsDying())
        {
            StartDeathSequence();
            return;
        }

        UpdateRedFlash(deltaTime);

        // Lining the three diamonds up grants a one-off bonus and stuns every Sno-Bee
        CheckDiamondLine();

        // Pengo may have smashed an egg; drop those from the reserve and the side counter
        for (auto it = m_eggBlocks.begin(); it != m_eggBlocks.end(); )
        {
            const auto& b = m_blocks[*it];
            if (b.pPooledBlock != nullptr && !b.pPooledBlock->HasEgg())
            {
                if (m_snoBeeReserve > 0) --m_snoBeeReserve;
                it = m_eggBlocks.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Let any in-progress hatch animation finish before starting the next
        if (AdvanceSpawnAnimation(deltaTime)) return;

        // Reserve cycle: sit idle, then flash all eggs for a couple of seconds, then hatch one
        if (!m_eggBlocks.empty())
        {
            if (!m_eggsFlashing)
            {
                m_hatchTimer -= deltaTime;
                if (m_hatchTimer <= 0.0f)
                {
                    m_eggsFlashing = true;       // begin the warning flash for every remaining egg
                    m_flashWindowTimer = PRE_SPAWN_FLASH;
                    m_redFlashTimer = 0.0f;
                    m_redFlashOn = true;
                }
            }
            else
            {
                m_flashWindowTimer -= deltaTime;
                if (m_flashWindowTimer <= 0.0f)
                {
                    m_eggsFlashing = false;
                    m_hatchTimer = HATCH_INTERVAL - PRE_SPAWN_FLASH; // idle before the next flash
                    BeginReserveHatch();
                }
            }
        }
    }

    void MazeDrawingComponent::StartDeathSequence()
    {
        m_phase = LevelPhase::DeathWipe;
        m_wipeProgress = 0.0f;

        // Remember how many enemies were around, then clear them off the field
        m_rememberedSnoBeeCount = ClearSnoBees();

        // Remember where the surviving ice blocks sit so we can rebuild the maze
        m_blockSnapshot = m_pIceBlockPool->GetActivePositions();

        // Remember the diamonds' positions too (they may have been pushed around)
        m_diamondSnapshot.clear();
        for (auto* d : m_diamondBlocks)
        {
            if (d) m_diamondSnapshot.push_back(d->GetLocalPosition());
        }

        ServiceLocator::get_sound_system().stop_music();
    }

    void MazeDrawingComponent::UpdateDeathWipe(float deltaTime)
    {
        // Slide the black curtain down over the whole playfield
        m_wipeProgress += WIPE_SPEED * deltaTime;

        const float fieldHeight = m_rows * m_blockSize;
        if (m_wipeProgress >= fieldHeight)
        {
            m_wipeProgress = fieldHeight;
            m_phase = LevelPhase::DeathHold;
            m_holdTimer = DEATH_HOLD_TIME;
        }
    }

    void MazeDrawingComponent::UpdateDeathHold(float deltaTime)
    {
        m_holdTimer -= deltaTime;
        if (m_holdTimer <= 0.0f)
        {
            BeginRespawn();
        }
    }

    void MazeDrawingComponent::BeginRespawn()
    {
        // Put the remembered ice blocks back where they were
        m_pIceBlockPool->Restore(m_blockSnapshot);

        // Restore re-uses pooled blocks, so re-mark the surviving eggs (and refresh their pointers)
        for (const int index : m_eggBlocks)
        {
            auto& b = m_blocks[index];
            IceBlock* block = m_pIceBlockPool->FindActiveAt(glm::vec3(GetScreenPos(b.r, b.c), 0.0f));
            b.pPooledBlock = block;
            if (block) block->SetHasEgg(true);
        }

        // Re-mark the diamonds at their remembered positions
        m_diamondBlocks.clear();
        for (const auto& pos : m_diamondSnapshot)
        {
            if (IceBlock* block = m_pIceBlockPool->FindActiveAt(pos))
            {
                block->SetDiamond(true);
                m_diamondBlocks.push_back(block);
            }
        }

        // Drop Pengo back at his original start tile, alive again
        if (m_pPengo)
        {
            m_pPengo->Respawn({ m_pengoSpawnPos.x, m_pengoSpawnPos.y, 0.0f });
        }

        // Hatch the same number of Sno-Bees that were alive before
        SpawnSnoBees(m_rememberedSnoBeeCount);

        m_phase = LevelPhase::Playing;
    }

    int MazeDrawingComponent::ClearSnoBees()
    {
        int count = 0;
        for (const auto& obj : m_scene.GetObjects())
        {
            if (auto* snoBee = dynamic_cast<SnoBeeCharacter*>(obj.get()))
            {
                if (!snoBee->IsMarkedForDelete())
                {
                    ++count;
                    snoBee->MarkForDelete();
                }
            }
        }
        return count;
    }

    void MazeDrawingComponent::SpawnSnoBees(int count)
    {
        if (count <= 0) return;

        // Respawn the survivors on free walkable tiles, never inside an ice block
        std::vector<int> freeCells;
        for (int i = 0; i < static_cast<int>(m_blocks.size()); ++i)
        {
            const auto& b = m_blocks[i];
            if (b.type != TileType::EMPTY) continue;
            if (GetScreenPos(b.r, b.c) == m_pengoSpawnPos) continue; // keep Pengo's tile clear
            freeCells.push_back(i);
        }
        if (freeCells.empty()) return;

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(freeCells.begin(), freeCells.end(), g);

        const SnoBeeType* basicType = TypeRegistry::GetInstance().GetSnoBeeType("Basic");
        for (int k = 0; k < count; ++k)
        {
            const auto& b = m_blocks[freeCells[k % freeCells.size()]];
            auto snoBee = std::make_unique<SnoBeeCharacter>(m_resourceManager, basicType);
            const glm::vec2 screenPos = GetScreenPos(b.r, b.c);
            snoBee->SetLocalPosition({ screenPos.x, screenPos.y, 0 });

            m_scene.AddSnoBee(snoBee.get());
            m_scene.Add(std::move(snoBee));
        }
    }

    bool MazeDrawingComponent::AdvanceSpawnAnimation(float deltaTime)
    {
        if (m_spawnStep == SpawnStep::None) return false;

        m_spawnAnimationTimer += deltaTime;
        const float currentFrameTime = (m_spawnStep == SpawnStep::SnoBeeSpawning) ? SNOBEE_SPAWN_FRAME_TIME : SPAWN_FRAME_TIME;

        if (m_spawnAnimationTimer >= currentFrameTime)
        {
            m_spawnAnimationTimer -= currentFrameTime;
            m_spawnAnimationFrame++;

            if (m_spawnStep == SpawnStep::IceBreaking)
            {
                if (m_spawnAnimationFrame >= 9)
                {
                    m_spawnStep = SpawnStep::SnoBeeSpawning;
                    m_spawnAnimationFrame = 0;
                }
            }
            else if (m_spawnStep == SpawnStep::SnoBeeSpawning)
            {
                if (m_spawnAnimationFrame >= 6)
                {
                    m_spawnStep = SpawnStep::Finished;

                    // Hatch a Sno-Bee at every animating block
                    const SnoBeeType* basicType = TypeRegistry::GetInstance().GetSnoBeeType("Basic");
                    for (const int index : m_spawnBlockIndices)
                    {
                        auto& b = m_blocks[index];
                        auto snoBee = std::make_unique<SnoBeeCharacter>(m_resourceManager, basicType);
                        const glm::vec2 screenPos = GetScreenPos(b.r, b.c);
                        snoBee->SetLocalPosition({ screenPos.x, screenPos.y, 0 });

                        m_scene.AddSnoBee(snoBee.get());
                        m_scene.Add(std::move(snoBee));
                        if (m_snoBeeReserve > 0) --m_snoBeeReserve; // one fewer waiting in reserve

                        b.removed = true;
                        b.isSpawning = false;
                    }

                    m_spawnBlockIndices.clear();
                    m_spawnStep = SpawnStep::None;

                    if (!m_hatchingReserve)
                    {
                        // First wave just finished: hand over to gameplay, then lay out diamonds and eggs
                        if (m_onFinished) m_onFinished(m_pengoSpawnPos);
                        m_phase = LevelPhase::Playing;
                        SetupDiamonds();
                        SetupReserveEggs();
                    }
                    m_hatchingReserve = false;
                }
            }
        }
        return true;
    }

    void MazeDrawingComponent::SetupReserveEggs()
    {
        // Scatter the remaining reserve across random surviving ice blocks; they flash red until they hatch
        std::vector<int> iceCells;
        for (int i = 0; i < static_cast<int>(m_blocks.size()); ++i)
        {
            const auto& b = m_blocks[i];
            if (b.removed || b.type != TileType::ICE || b.isSpawning) continue;
            if (b.pPooledBlock && b.pPooledBlock->IsDiamond()) continue; // don't put an egg in a diamond
            iceCells.push_back(i);
        }
        if (iceCells.empty()) return;

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(iceCells.begin(), iceCells.end(), g);

        const int eggCount = std::min(m_snoBeeReserve, static_cast<int>(iceCells.size()));
        for (int i = 0; i < eggCount; ++i)
        {
            auto& b = m_blocks[iceCells[i]];
            m_eggBlocks.push_back(iceCells[i]);
            // Flag the ice block so wandering Sno-Bees won't crush their un-hatched siblings
            if (b.pPooledBlock) b.pPooledBlock->SetHasEgg(true);
        }
    }

    void MazeDrawingComponent::BeginReserveHatch()
    {
        if (m_eggBlocks.empty()) return;

        // Take one egg and hatch it with the same animation as the opening wave
        const int index = m_eggBlocks.back();
        m_eggBlocks.pop_back();

        auto& b = m_blocks[index];
        b.isSpawning = true;
        // Drop the ice block so the Sno-Bee hatches out of it instead of spawning inside it
        if (b.pPooledBlock)
        {
            m_pIceBlockPool->Release(b.pPooledBlock);
            b.pPooledBlock = nullptr;
        }

        m_spawnBlockIndices.clear();
        m_spawnBlockIndices.push_back(index);
        m_hatchingReserve = true;
        m_spawnStep = SpawnStep::IceBreaking;
        m_spawnAnimationFrame = 0;
        m_spawnAnimationTimer = 0.0f;
    }

    void MazeDrawingComponent::SetupDiamonds()
    {
        // Pick three surviving ice blocks and turn them into push-only diamonds
        std::vector<int> iceCells;
        for (int i = 0; i < static_cast<int>(m_blocks.size()); ++i)
        {
            const auto& b = m_blocks[i];
            if (b.removed || b.type != TileType::ICE || b.isSpawning) continue;
            if (b.pPooledBlock == nullptr) continue;
            iceCells.push_back(i);
        }
        if (static_cast<int>(iceCells.size()) < DIAMOND_COUNT) return;

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(iceCells.begin(), iceCells.end(), g);

        m_diamondBlocks.clear();
        for (int i = 0; i < DIAMOND_COUNT; ++i)
        {
            auto& b = m_blocks[iceCells[i]];
            b.pPooledBlock->SetDiamond(true);
            m_diamondBlocks.push_back(b.pPooledBlock);
        }
    }

    void MazeDrawingComponent::CheckDiamondLine()
    {
        if (static_cast<int>(m_diamondBlocks.size()) < DIAMOND_COUNT) return;

        // Wait until every diamond has come to rest; a sliding one only passes through its tile
        for (IceBlock* d : m_diamondBlocks)
        {
            if (d && d->IsSliding()) return;
        }

        const auto& grid = ServiceLocator::get_collision_grid();
        std::array<std::pair<int, int>, DIAMOND_COUNT> cells{}; // (row, col) of each diamond
        for (int i = 0; i < DIAMOND_COUNT; ++i)
        {
            IceBlock* d = m_diamondBlocks[i];
            if (d == nullptr || !d->IsActive())
            {
                m_diamondLineActive = false;
                return;
            }
            cells[i] = grid.WorldToGrid(d->GetWorldPosition());
        }

        // Three in a row, either all on one row with consecutive columns or one column with consecutive rows
        bool aligned = false;
        if (cells[0].first == cells[1].first && cells[1].first == cells[2].first)
        {
            std::array<int, 3> cols{ cells[0].second, cells[1].second, cells[2].second };
            std::sort(cols.begin(), cols.end());
            aligned = (cols[1] == cols[0] + 1 && cols[2] == cols[1] + 1);
        }
        else if (cells[0].second == cells[1].second && cells[1].second == cells[2].second)
        {
            std::array<int, 3> rows{ cells[0].first, cells[1].first, cells[2].first };
            std::sort(rows.begin(), rows.end());
            aligned = (rows[1] == rows[0] + 1 && rows[2] == rows[1] + 1);
        }

        if (aligned && !m_diamondLineActive)
        {
            // Award the bonus and freeze every Sno-Bee, but only once per line-up
            m_diamondLineActive = true;
            if (m_pPengo) m_pPengo->AddScore(DIAMOND_BONUS);
            StunAllSnoBees(DIAMOND_STUN_TIME);
        }
        else if (!aligned)
        {
            m_diamondLineActive = false;
        }
    }

    void MazeDrawingComponent::StunAllSnoBees(float duration)
    {
        for (const auto& obj : m_scene.GetObjects())
        {
            if (auto* snoBee = dynamic_cast<SnoBeeCharacter*>(obj.get()))
            {
                if (!snoBee->IsMarkedForDelete())
                {
                    snoBee->Stun(duration);
                }
            }
        }
    }

    void MazeDrawingComponent::UpdateRedFlash(float deltaTime)
    {
        m_redFlashTimer += deltaTime;
        if (m_redFlashTimer >= RED_FLASH_INTERVAL)
        {
            m_redFlashTimer -= RED_FLASH_INTERVAL;
            m_redFlashOn = !m_redFlashOn;
        }
    }

    void MazeDrawingComponent::Render() const
    {
        auto& renderer = Renderer::GetInstance();
        const auto& worldPos = GetOwner()->GetWorldPosition();

        // Render background
        const Rect srcBackground = { 0.0f, 0.0f, 224.0f, 256.0f };
        const float scale = m_blockSize / 16.0f;
        const float bgWidth = 224.0f * scale;
        const float bgHeight = 256.0f * scale;
        renderer.RenderTexture(*m_backgroundTexture, srcBackground, worldPos.x + 16.0f, worldPos.y + 16.0f, bgWidth, bgHeight);

        for (const auto& b : m_blocks)
        {
            if (!g_ShowFullMaze && b.removed) continue;
            const glm::vec2 screenPos = GetScreenPos(b.r, b.c);
            const float dstX = worldPos.x + screenPos.x;
            const float dstY = worldPos.y + screenPos.y;

            if (b.isSpawning)
            {
                Rect animSrc;
                if (m_spawnStep == SpawnStep::IceBreaking)
                {
                    // Ice breaking is likely 9 frames. 
                    animSrc = { m_spawnAnimationFrame * 16.0f, 48.0f, 16.0f, 16.0f };
                    renderer.RenderTexture(*m_miscTexture, animSrc, dstX, dstY, m_blockSize, m_blockSize);
                }
                else // SnoBeeSpawning
                {
                    // SnoBee spawning frames in pengo.png
                    // they are 6 frames.
                    animSrc = { (8 + m_spawnAnimationFrame) * 16.0f, 8 * 16.0f, 16.0f, 16.0f };
                    renderer.RenderTexture(*m_pengoTexture, animSrc, dstX, dstY, m_blockSize, m_blockSize);
                }
            }
        }

        m_pIceBlockPool->Render();

        // Just before a hatch, every remaining egg flashes its red square together
        if (m_phase == LevelPhase::Playing && m_eggsFlashing && m_redFlashOn)
        {
            const Rect redSrc = { 32.0f, 0.0f, 16.0f, 16.0f }; // red cell in misc.png's top row
            for (const int index : m_eggBlocks)
            {
                const auto& b = m_blocks[index];
                if (b.removed) continue;
                const glm::vec2 screenPos = GetScreenPos(b.r, b.c);
                renderer.RenderTexture(*m_miscTexture, redSrc, worldPos.x + screenPos.x, worldPos.y + screenPos.y, m_blockSize, m_blockSize);
            }
        }

        // Black curtain that swallows the playfield while Pengo is dying (drawn under Pengo)
        if (m_phase == LevelPhase::DeathWipe || m_phase == LevelPhase::DeathHold)
        {
            const float fieldWidth = m_cols * m_blockSize;
            renderer.RenderFilledRect(worldPos.x, worldPos.y, fieldWidth, m_wipeProgress, Color{ 0, 0, 0, 255 });
        }
    }

    glm::vec2 MazeDrawingComponent::GetScreenPos(int r, int c) const
    {
        // Simple linear transformation from grid to screen pixels
        // m_offsetX and m_offsetY handle the "pushing down and centering"
        const float x = m_offsetX + c * m_blockSize;
        const float y = m_offsetY + r * m_blockSize;
        return { x, y };
    }

    std::unique_ptr<Component> MazeDrawingComponent::Clone(GameObject* pOwner) const
    {
        auto clone = std::make_unique<MazeDrawingComponent>(pOwner, m_scene, m_resourceManager, m_levelFile, m_onFinished);
        clone->m_blocks = m_blocks;
        clone->m_removalOrder = m_removalOrder;
        clone->m_isFinished = m_isFinished;
        clone->m_timer = m_timer;
        clone->m_removedStep = m_removedStep;
        clone->m_spawnStep = m_spawnStep;
        clone->m_spawnBlockIndices = m_spawnBlockIndices;
        clone->m_spawnAnimationFrame = m_spawnAnimationFrame;
        clone->m_spawnAnimationTimer = m_spawnAnimationTimer;
        clone->m_pengoTexture = m_pengoTexture;
        
        // I Don't clone the pool, each instance needs its own pool

        return clone;
    }
}
