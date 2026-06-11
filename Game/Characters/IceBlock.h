#pragma once
#include "GameObject.h"
#include "Renderer.h" // for dae::Rect

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
            EggBreaking, // Pengo crushed a reserve egg: special break animation, then a score
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

        // An un-hatched reserve Sno-Bee sits inside this block; Sno-Bees must not crush it
        void SetHasEgg(bool hasEgg) { m_hasEgg = hasEgg; }
        bool HasEgg() const { return m_hasEgg; }

        // A diamond block can be pushed but never crushed (by Pengo or Sno-Bees)
        void SetDiamond(bool isDiamond) { m_isDiamond = isDiamond; }
        bool IsDiamond() const { return m_isDiamond; }

    private:
        bool m_isActive{ false };
        bool m_hasEgg{ false };
        bool m_isDiamond{ false };
        RenderComponent* m_pRenderComponent{ nullptr };

        State m_state{ State::Idle };
        glm::vec2 m_slideDirection{ 0.0f, 0.0f };

        float m_crushTimer{ 0.0f };
        int m_crushFrame{ 0 };
        std::shared_ptr<Texture2D> m_miscTexture;

        float m_scoreTimer{ 0.0f };
        std::shared_ptr<Texture2D> m_scoresTexture;
        Rect m_scoreSrcRect{ 16.0f, 16.0f, 16.0f, 16.0f }; // which score sprite to flash
        bool m_removeAfterScore{ false };                  // disappear after the score (egg) vs settle as ice

        static constexpr float CRUSH_FRAME_TIME = 0.1f;
        static constexpr int CRUSH_FRAMES = 9;
        static constexpr int EGG_BREAK_FRAMES = 6;         // misc.png row 5 holds the egg-break frames
        static constexpr float EGG_BREAK_ROW_Y = 64.0f;
        static constexpr float EGG_BREAK_FRAME_TIME = 0.18f; // a touch slower than a normal shatter
        static constexpr float SLIDE_SPEED = 250.0f;
        static constexpr float SCORE_DISPLAY_TIME = 3.0f;
    };
}
