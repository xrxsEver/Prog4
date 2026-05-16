#pragma once
#include "GameObject.h"

namespace dae
{
    class RenderComponent;
    class ResourceManager;

    class IceBlock final : public GameObject
    {
    public:
        explicit IceBlock(ResourceManager& resourceManager);
        ~IceBlock() override = default;

        void Update(float deltaTime);
        void Render() const;

        void SetActive(bool active);
        bool IsActive() const { return m_isActive; }

        void Reset();

    private:
        bool m_isActive{ false };
        RenderComponent* m_pRenderComponent{ nullptr };
    };
}
