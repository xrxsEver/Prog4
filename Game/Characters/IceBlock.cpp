#include "IceBlock.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "GridObjectComponent.h"

namespace dae
{
    IceBlock::IceBlock(ResourceManager& resourceManager)
        : GameObject("IceBlock")
    {
        m_pRenderComponent = AddComponent<RenderComponent>(resourceManager);
        m_pRenderComponent->SetTexture("iceblock.png");
        m_pRenderComponent->SetRenderSize(32.0f, 32.0f);
        
        AddComponent<GridObjectComponent>();

        SetActive(false);
    }

    void IceBlock::SetActive(bool active)
    {
        m_isActive = active;
        if (auto pGridComp = GetComponent<GridObjectComponent>())
        {
            if (active)
            {
                pGridComp->Enable();
            }
            else
            {
                pGridComp->Disable();
            }
        }
    }

    void IceBlock::Render() const
    {
        if (m_isActive)
        {
            GameObject::Render();
        }
    }

    void IceBlock::Update(float deltaTime)
    {
        if (m_isActive)
        {
            GameObject::Update(deltaTime);
        }
    }

    void IceBlock::Reset()
    {
        SetActive(false);
        SetPosition(-1000.0f, -1000.0f);
    }
}
