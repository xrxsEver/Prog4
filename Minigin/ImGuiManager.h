#pragma once
#include <SDL3/SDL.h>
#include "Singleton.h"

namespace dae
{
    class GameObject;

    class ImGuiManager final : public Singleton<ImGuiManager>
    {
    public:
        void Init(SDL_Window *window, SDL_Renderer *renderer);
        void ShutDown();

        void BeginFrame();
        void EndFrame(SDL_Renderer *renderer);

    private:
        friend class Singleton<ImGuiManager>;
        ImGuiManager() = default;

        void RenderOverlay();
        void RenderReopenButton();
        void RenderSceneHierarchy();
        void RenderHierarchyNode(GameObject &gameObject);
        void RenderInspector() const;
        void RenderInputMonitor() const;
        void ApplyModernStyle() const;
        bool IsSelectedObjectAlive() const;

        int m_frameCount{};
        float m_elapsedTime{};
        float m_fps{};
        GameObject *m_pSelectedGameObject{};
        bool m_isWindowOpen{true};
    };
}
