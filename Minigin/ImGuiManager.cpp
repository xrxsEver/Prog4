#include "ImGuiManager.h"
#include "Component.h"
#include "GameTime.h"
#include "GameObject.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include <cmath>
#include <cstddef>
#include <string>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

namespace
{
    float StickMagnitude(const std::int16_t x, const std::int16_t y)
    {
        return std::sqrt(static_cast<float>(x) * static_cast<float>(x) + static_cast<float>(y) * static_cast<float>(y));
    }

    void RenderKeyIndicator(const char *label, const bool isPressed)
    {
        if (isPressed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.19f, 0.51f, 0.42f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.61f, 0.49f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.43f, 0.36f, 1.0f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.11f, 0.13f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.18f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.22f, 0.27f, 1.0f));
        }

        ImGui::Button(label, ImVec2(56.0f, 36.0f));
        ImGui::PopStyleColor(3);
    }
}

void dae::ImGuiManager::Init(SDL_Window *window, SDL_Renderer *renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
#if __EMSCRIPTEN__
    io.IniFilename = NULL;
#endif

    ApplyModernStyle();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void dae::ImGuiManager::ApplyModernStyle() const
{
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 14.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 10.0f;
    style.GrabRounding = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.10f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Header] = ImVec4(0.14f, 0.18f, 0.22f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.25f, 0.30f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.21f, 0.33f, 0.35f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.19f, 0.24f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.24f, 0.30f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.13f, 0.16f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.22f, 0.26f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.94f, 0.96f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.54f, 0.59f, 0.64f, 1.0f);
}

void dae::ImGuiManager::ShutDown()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void dae::ImGuiManager::BeginFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RenderOverlay();
}

void dae::ImGuiManager::EndFrame(SDL_Renderer *renderer)
{
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}

void dae::ImGuiManager::RenderOverlay()
{
    // Calculate FPS
    float deltaTime = GameTime::GetInstance().GetDeltaTime();
    m_elapsedTime += deltaTime;
    ++m_frameCount;

    if (m_elapsedTime >= 1.0f)
    {
        m_fps = static_cast<float>(m_frameCount) / m_elapsedTime;
        m_frameCount = 0;
        m_elapsedTime = 0.f;
    }

    if (!IsSelectedObjectAlive())
    {
        m_pSelectedGameObject = nullptr;
    }

    if (!m_isWindowOpen)
    {
        RenderReopenButton();
        return;
    }

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    constexpr float panelWidth = 360.0f;
    constexpr float margin = 12.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - panelWidth - margin, viewport->WorkPos.y + margin), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->WorkSize.y - margin * 2.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.92f);

    if (ImGui::Begin("Debug Overlay", &m_isWindowOpen))
    {
        ImGui::Text("Debug Overlay");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90.0f);
        ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.68f, 1.0f), "%.1f FPS", m_fps);

        RenderSceneHierarchy();
        ImGui::Spacing();
        RenderInspector();
        ImGui::Spacing();
        RenderInputMonitor();
        //sImGui::ShowDemoWindow();
    }
    ImGui::End();
}

void dae::ImGuiManager::RenderReopenButton()
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 104.0f, viewport->WorkPos.y + 16.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("OverlayReopen", nullptr, flags))
    {
        if (ImGui::Button("Open UI", ImVec2(84.0f, 30.0f)))
        {
            m_isWindowOpen = true;
        }
    }
    ImGui::End();
}

bool dae::ImGuiManager::IsSelectedObjectAlive() const
{
    if (m_pSelectedGameObject == nullptr)
    {
        return true;
    }

    const auto &scenes = SceneManager::GetInstance().GetScenes();
    for (const auto &scene : scenes)
    {
        for (const auto &object : scene->GetObjects())
        {
            if (object.get() == m_pSelectedGameObject)
            {
                return true;
            }
        }
    }

    return false;
}

void dae::ImGuiManager::RenderSceneHierarchy()
{
    if (!ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    const auto &scenes = SceneManager::GetInstance().GetScenes();
    for (size_t index{}; index < scenes.size(); ++index)
    {
        const auto &scene = scenes[index];
        const std::string label = "Scene " + std::to_string(index);
        if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            for (const auto &object : scene->GetObjects())
            {
                if (object->GetParent() == nullptr)
                {
                    RenderHierarchyNode(*object);
                }
            }
            ImGui::TreePop();
        }
    }
}

void dae::ImGuiManager::RenderHierarchyNode(GameObject &gameObject)
{
    const auto &children = gameObject.GetChildren();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (children.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    if (m_pSelectedGameObject == &gameObject)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool isOpen = ImGui::TreeNodeEx(gameObject.GetName().c_str(), flags);
    if (ImGui::IsItemClicked())
    {
        m_pSelectedGameObject = &gameObject;
    }

    if (!children.empty() && isOpen)
    {
        for (auto *child : children)
        {
            if (child != nullptr)
            {
                RenderHierarchyNode(*child);
            }
        }
        ImGui::TreePop();
    }
}

void dae::ImGuiManager::RenderInspector() const
{
    if (!ImGui::CollapsingHeader("Selection", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (m_pSelectedGameObject == nullptr)
    {
        ImGui::TextDisabled("Select a game object from the hierarchy.");
        return;
    }

    ImGui::TextUnformatted(m_pSelectedGameObject->GetName().c_str());
    const auto &localPosition = m_pSelectedGameObject->GetLocalPosition();
    const auto &worldPosition = m_pSelectedGameObject->GetWorldPosition();
    ImGui::Text("Local: %.1f, %.1f, %.1f", localPosition.x, localPosition.y, localPosition.z);
    ImGui::Text("World: %.1f, %.1f, %.1f", worldPosition.x, worldPosition.y, worldPosition.z);
    ImGui::Text("Children: %zu", m_pSelectedGameObject->GetChildren().size());

    ImGui::Spacing();
    ImGui::TextDisabled("Components");
    ImGui::Separator();

    const auto &components = m_pSelectedGameObject->GetComponents();
    if (components.empty())
    {
        ImGui::TextDisabled("No components attached.");
        return;
    }

    for (const auto &component : components)
    {
        if (ImGui::CollapsingHeader(component->GetDebugName(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            component->DrawInspector();
        }
    }
}

void dae::ImGuiManager::RenderInputMonitor() const
{
    if (!ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    const auto &input = InputManager::GetInstance();

    ImGui::TextUnformatted("Pengo keyboard state");
    const float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + 62.0f);
    RenderKeyIndicator("W", input.IsKeyboardPressed(SDL_SCANCODE_W));
    ImGui::SetCursorPosX(cursorX);
    RenderKeyIndicator("A", input.IsKeyboardPressed(SDL_SCANCODE_A));
    ImGui::SameLine();
    RenderKeyIndicator("S", input.IsKeyboardPressed(SDL_SCANCODE_S));
    ImGui::SameLine();
    RenderKeyIndicator("D", input.IsKeyboardPressed(SDL_SCANCODE_D));

    ImGui::Spacing();

    if (const auto gamepadState = input.GetGamepadState(InputManager::AnyGamepad))
    {
        const float leftMagnitude = StickMagnitude(gamepadState->leftThumbX, gamepadState->leftThumbY);
        const float rightMagnitude = StickMagnitude(gamepadState->rightThumbX, gamepadState->rightThumbY);

        ImGui::Text("Controller: %.*s slot %u", static_cast<int>(gamepadState->backend.size()), gamepadState->backend.data(), gamepadState->playerIndex);
        ImGui::Text("Packet: %u", gamepadState->packetNumber);
        ImGui::Text("Buttons: 0x%04X", gamepadState->buttons);
        ImGui::Text("Left stick: %d, %d", gamepadState->leftThumbX, gamepadState->leftThumbY);
        ImGui::Text("Left deadzone: %.0f / %d", leftMagnitude, gamepadState->leftDeadzone);
        ImGui::Text("Right stick: %d, %d", gamepadState->rightThumbX, gamepadState->rightThumbY);
        ImGui::Text("Right deadzone: %.0f / %d", rightMagnitude, gamepadState->rightDeadzone);
        ImGui::Text("Triggers: %u / %u  threshold %u", gamepadState->leftTrigger, gamepadState->rightTrigger, gamepadState->triggerThreshold);
        ImGui::TextDisabled("SnoBee uses both the D-pad and the left stick.");
    }
    else
    {
        ImGui::TextDisabled("No controller connected.");
    }
}
