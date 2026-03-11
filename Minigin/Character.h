#pragma once

#include <string>
#include "GameObject.h"

namespace dae
{
    class Character : public GameObject
    {
    public:
        explicit Character(std::string name);

    protected:
        void InitializeSprite(float sourceX, float sourceY);

    private:
        static constexpr float m_spriteSize{16.0f};
        static constexpr float m_displaySize{32.0f};
    };
}