#pragma once
#include <SDL3/SDL.h>
#include "Singleton.h"
#include <string>
#include <vector>
#include <functional>

namespace dae
{
    class GameObject;
    class InputManager;
    class SceneManager;

    class ImGuiManager final : public Singleton<ImGuiManager>
    {
    public:
        void Init(SDL_Window *window, SDL_Renderer *renderer, SceneManager &sceneManager, InputManager &inputManager);
        void ShutDown();

        void BeginFrame();
        void EndFrame(SDL_Renderer *renderer);
        void AddCustomTab(const std::string& name, std::function<void()> renderFunc);

    private:
        friend class Singleton<ImGuiManager>;
        ImGuiManager() = default;

        void RenderOverlay();
        void RenderReopenButton();
        void RenderSceneHierarchy();
        void RenderHierarchyNode(GameObject &gameObject);
        void RenderInspector() const;
        void RenderInputMonitor() const;
        void RenderEventMonitor() const;
        void RenderAudioLog() const;
        void RenderEngineTabs();
        void ApplyModernStyle() const;
        bool IsSelectedObjectAlive() const;

        int m_frameCount{};
        float m_elapsedTime{};
        float m_fps{};
        GameObject *m_pSelectedGameObject{};
        bool m_isWindowOpen{true};
        float m_sceneHierarchyHeight{260.0f};
        bool m_confirmResetAllStats{};
        std::string m_achievementActionFeedback{};
        SceneManager *m_pSceneManager{};
        InputManager *m_pInputManager{};
        std::vector<std::pair<std::string, std::function<void()>>> m_customTabs;
    };
}
