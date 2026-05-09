#include "MazeDrawingComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "MazeGenerator.h"
#include <algorithm>
#include <random>

namespace dae
{
    bool MazeDrawingComponent::g_ShowFullMaze = false;

    MazeDrawingComponent::MazeDrawingComponent(GameObject* owner, ResourceManager& resourceManager, std::function<void()> onFinished)
        : Component(owner)
        , m_resourceManager(resourceManager)
        , m_onFinished(onFinished)
    {
        m_backgroundTexture = m_resourceManager.LoadTexture("Playfield.png");
        m_iceBlockTexture = m_resourceManager.LoadTexture("iceblock.png");

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
                m_blocks.push_back({ r, c, false, result.grid[r][c] });
            }
        }

        // The removal order follows the carving sequence
        for (const auto& pos : result.carvingSequence)
        {
            // Find index of block at pos
            int index = pos.first * m_cols + pos.second;
            m_removalOrder.push_back(index);
        }

        // Start playing the drawing sound as music so we can stop it
        ServiceLocator::get_sound_system().play_music("Sounds/Drawing Maze.mp3", 0.6f, true);
    }

    void MazeDrawingComponent::Update(float deltaTime)
    {
        if (m_isFinished) return;

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
                if (m_onFinished) m_onFinished();
            }
        }
    }

    void MazeDrawingComponent::Render() const
    {
        auto& renderer = Renderer::GetInstance();
        const auto& worldPos = GetOwner()->GetWorldPosition();

        // 1. Render the background
        SDL_FRect srcBackground = { 0.0f, 0.0f, 224.0f, 256.0f };
        float scale = m_blockSize / 16.0f;
        float bgWidth = 224.0f * scale;
        float bgHeight = 256.0f * scale;
        renderer.RenderTexture(*m_backgroundTexture, srcBackground, worldPos.x, worldPos.y, bgWidth, bgHeight);

        // 2. Render Grid Tiles (Blocks)
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

            // Actual Render Call
            renderer.RenderTexture(*m_iceBlockTexture, srcRect, dstX, dstY, m_blockSize, m_blockSize);
        }
    }

    glm::vec2 MazeDrawingComponent::GetScreenPos(int r, int c) const
    {
        // Simple linear transformation from grid to screen pixels
        // m_offsetX and m_offsetY handle the "pushing down and centering"
        return { m_offsetX + (float)c * m_blockSize, m_offsetY + (float)r * m_blockSize };
    }
}
