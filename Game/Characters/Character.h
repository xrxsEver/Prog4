#pragma once

#include <string>
#include "GameObject.h"
#include "Subject.h"
#include "RenderComponent.h" // Include RenderComponent header

namespace dae
{
    class ResourceManager;

    class Character : public GameObject, public Subject
    {
    public:
        Character(std::string name, ResourceManager &resourceManager);
        void LoseLife();
        void AddScore(int points);

        // Public data members (C.131) - no invariants to protect
        int health{m_startHealth};
        int score{0};

        // Public methods to configure the sprite
        void SetSpriteTexture(std::string_view filename);
        void SetSpriteSourceRect(float x, float y, float w, float h);
        void SetSpriteRenderSize(float w, float h);

    protected:
        void InitializeSprite(float sourceX, float sourceY); // This method will now configure the m_pRenderComponent

    private:
        static constexpr float m_spriteSize{16.0f};
        static constexpr float m_displaySize{32.0f};
        static constexpr int m_startHealth{5};

    protected:
        ResourceManager &m_resourceManager;
        RenderComponent* m_pRenderComponent; // Cached pointer to the RenderComponent
    };
}