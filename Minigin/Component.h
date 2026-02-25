#pragma once
#include <memory>

namespace dae
{
    class GameObject;
    class Texture2D;

    class Component
    {
    public:
        virtual void Update() {}
        virtual void FixedUpdate() {}
        virtual void Render() const {}

        virtual ~Component() = default;

        Component(const Component &) = delete;
        Component(Component &&) = delete;
        Component &operator=(const Component &) = delete;
        Component &operator=(Component &&) = delete;

        GameObject *GetOwner() const { return m_pOwner; }

    protected:
        explicit Component(GameObject *pOwner) : m_pOwner(pOwner) {}
        void RenderTextureAtOwnerPosition(const std::shared_ptr<Texture2D> &texture) const;

    private:
        GameObject *m_pOwner;
    };
}
