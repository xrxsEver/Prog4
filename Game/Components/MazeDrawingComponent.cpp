#include "MazeDrawingComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "MazeGenerator.h"
#include "Scene.h"
#include "SnoBeeCharacter.h"
#include <algorithm>
#include <random>

namespace dae
{
    bool MazeDrawingComponent::g_ShowFullMaze = false;

    MazeDrawingComponent::MazeDrawingComponent(GameObject* owner, Scene& scene, ResourceManager& resourceManager, std::function<void()> onFinished)
        : Component(owner)
        , m_scene(scene)
        , m_resourceManager(resourceManager)
        , m_onFinished(onFinished)
    {
        m_backgroundTexture = m_resourceManager.LoadTexture("Playfield.png");
        m_iceBlockTexture = m_resourceManager.LoadTexture("iceblock.png");
        m_miscTexture = m_resourceManager.LoadTexture("misc.png");
        m_pengoTexture = m_resourceManager.LoadTexture("pengo.png");

        // Generate the maze
        MazeGenerator generator;
        MazeConfig config;
        config.rows = m_rows;
        config.cols = m_cols;
        config.emptyDensity = 0.4f;
        auto result = generator.Generate(config);

        // Initialize grid
        m_blocks.reserve(m_rows * m_cols);
        for (int r = 0; r < m_rows; ++r)
        {
            for (int c = 0; c < m_cols; ++c)
            {
                // All tiles start as ICE (or WALL if border)
                // In Pengo, the "drawing" reveals the maze paths by removing ice blocks.
                m_blocks.push_back({ r, c, false, result.grid[r][c], false, 0, 0.0f });
            }
        }

        // The removal order follows the carving sequence
        for (const auto& pos : result.carvingSequence)
        {
            // Find index of block at pos
            int index = pos.first * m_cols + pos.second;
            m_removalOrder.push_back(index);
        }
    }

    void MazeDrawingComponent::Update(float deltaTime)
    {
        if (m_isFinished)
        {
            if (m_spawnStep == SpawnStep::None) return;

            m_spawnAnimationTimer += deltaTime;
            float currentFrameTime = (m_spawnStep == SpawnStep::SnoBeeSpawning) ? SNOBEE_SPAWN_FRAME_TIME : SPAWN_FRAME_TIME;

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
                        
                        // Actually spawn them
                        for (int index : m_spawnBlockIndices)
                        {
                            auto& b = m_blocks[index];
                            auto snoBee = std::make_unique<SnoBeeCharacter>(m_resourceManager);
                            glm::vec2 screenPos = GetScreenPos(b.r, b.c);
                            snoBee->SetLocalPosition({ screenPos.x, screenPos.y, 0 });
                            m_scene.Add(std::move(snoBee));
                            
                            b.removed = true;
                            b.isSpawning = false;
                        }
                        
                        if (m_onFinished) m_onFinished();
                        m_spawnStep = SpawnStep::None;
                    }
                }
            }
            return;
        }

        m_timer += deltaTime;
        if (m_timer >= m_blockRemoveInterval)
        {
            m_timer = 0.0f;
            if (m_removedStep < (int)m_removalOrder.size())
            {
                int index = m_removalOrder[m_removedStep];
                m_blocks[index].removed = true;
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
                for (int i = 0; i < (int)m_blocks.size(); ++i)
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
                        m_spawnBlockIndices.push_back(iceBlockIndices[i]);
                        m_blocks[iceBlockIndices[i]].isSpawning = true;
                    }
                    m_spawnStep = SpawnStep::IceBreaking;
                    m_spawnAnimationFrame = 0;
                    m_spawnAnimationTimer = 0.0f;
                }
                else
                {
                    if (m_onFinished) m_onFinished();
                }
            }
        }
    }

    void MazeDrawingComponent::Render() const
    {
        auto& renderer = Renderer::GetInstance();
        const auto& worldPos = GetOwner()->GetWorldPosition();

        // Render background
        SDL_FRect srcBackground = { 0.0f, 0.0f, 224.0f, 256.0f };
        float scale = m_blockSize / 16.0f;
        float bgWidth = 224.0f * scale;
        float bgHeight = 256.0f * scale;
        renderer.RenderTexture(*m_backgroundTexture, srcBackground, worldPos.x, worldPos.y, bgWidth, bgHeight);

        // Render Grid Tiles (Blocks)
        // The ice block texture is 16x16
        SDL_FRect srcRect = { 0.0f, 0.0f, 16.0f, 16.0f };

        // This loop separates Logical Grid (r, c) from Screen Rendering (pixels)
        for (const auto& b : m_blocks)
        {
            // Visibility Check: Skip removed blocks unless debug is on
            if (!g_ShowFullMaze && b.removed) continue;

            // Separation of Logical Grid Coordinates from Screen Rendering Coordinates
            glm::vec2 screenPos = GetScreenPos(b.r, b.c);
            float dstX = worldPos.x + screenPos.x;
            float dstY = worldPos.y + screenPos.y;

            if (b.isSpawning)
            {
                SDL_FRect animSrc;
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
            else
            {
                // Actual Render Call
                renderer.RenderTexture(*m_iceBlockTexture, srcRect, dstX, dstY, m_blockSize, m_blockSize);
            }
        }
    }

    glm::vec2 MazeDrawingComponent::GetScreenPos(int r, int c) const
    {
        // Simple linear transformation from grid to screen pixels
        // m_offsetX and m_offsetY handle the "pushing down and centering"
        float x = m_offsetX + c * m_blockSize;
        float y = m_offsetY + r * m_blockSize;
        return { x, y };
    }

    std::unique_ptr<Component> MazeDrawingComponent::Clone(GameObject* pOwner) const
    {
        auto clone = std::make_unique<MazeDrawingComponent>(pOwner, m_scene, m_resourceManager, m_onFinished);
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
        return clone;
    }
}
