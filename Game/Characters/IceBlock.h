#pragma once
#include "GameObject.h"

namespace dae
{
    class RenderComponent;
    class ResourceManager;
    class Texture2D;

    class IceBlock final : public GameObject
    {
    public:
        enum class State
        {
            Idle,
            Sliding,
            Crushing,
            ShowingScore // flashing the kill score before settling as a normal block
        };

        explicit IceBlock(ResourceManager& resourceManager);
        ~IceBlock() override = default;

        void Update(float deltaTime);
        void Render() const;

        void SetActive(bool active);
        bool IsActive() const { return m_isActive; }

        void Reset();

        void Crush();
        void Slide(const glm::vec2& direction);

        bool IsSliding() const { return m_state == State::Sliding; }
        const glm::vec2& GetSlideDirection() const { return m_slideDirection; }

    private:
        bool m_isActive{ false };
        RenderComponent* m_pRenderComponent{ nullptr };

        State m_state{ State::Idle };
        glm::vec2 m_slideDirection{ 0.0f, 0.0f };

        float m_crushTimer{ 0.0f };
        int m_crushFrame{ 0 };
        std::shared_ptr<Texture2D> m_miscTexture;

        float m_scoreTimer{ 0.0f };
        std::shared_ptr<Texture2D> m_scoresTexture;

        static constexpr float CRUSH_FRAME_TIME = 0.1f;
        static constexpr int CRUSH_FRAMES = 9;
        static constexpr float SLIDE_SPEED = 250.0f;
        static constexpr float SCORE_DISPLAY_TIME = 2.0f;
    };
}
