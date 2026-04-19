#include "ImGuiManager.h"
#include "Component.h"
#include "Achievements.h"
#include "GameTime.h"
#include "GameObject.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Subject.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

    const char *ToText(const dae::GameEvent event)
    {
        switch (event)
        {
        case dae::GameEvent::PlayerDied:
            return "PlayerDied";
        case dae::GameEvent::PointsGained:
            return "PointsGained";
        default:
            return "Unknown";
        }
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

void dae::ImGuiManager::Init(SDL_Window *window, SDL_Renderer *renderer, SceneManager &sceneManager, InputManager &inputManager)
{
    m_pSceneManager = &sceneManager;
    m_pInputManager = &inputManager;

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
    style.WindowRounding = 12.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.TabRounding = 7.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.96f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.10f, 0.12f, 0.95f);
    colors[ImGuiCol_Header] = ImVec4(0.16f, 0.21f, 0.26f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.29f, 0.36f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.36f, 0.42f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.18f, 0.23f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.25f, 0.31f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.32f, 0.39f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.14f, 0.18f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.20f, 0.25f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.28f, 0.34f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.12f, 0.16f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.16f, 0.22f, 0.28f, 1.0f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.29f, 0.36f, 1.0f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.10f, 0.13f, 1.0f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.20f, 0.25f, 1.0f);
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
    constexpr float panelWidth = 460.0f;
    constexpr float margin = 12.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - panelWidth - margin, viewport->WorkPos.y + margin), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, viewport->WorkSize.y - margin * 2.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.92f);

    if (ImGui::Begin("Debug Overlay", &m_isWindowOpen))
    {
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90.0f);
        ImGui::TextColored(ImVec4(0.35f, 0.82f, 0.68f, 1.0f), "%.1f FPS", m_fps);
        ImGui::Separator();

        RenderEngineTabs();
    }
    ImGui::End();
}

void dae::ImGuiManager::RenderEngineTabs()
{
    constexpr ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_FittingPolicyResizeDown;
    if (!ImGui::BeginTabBar("DebugWorkbenchTabs", tabFlags))
    {
        return;
    }

    if (ImGui::BeginTabItem("Scene"))
    {
        const float availableHeight = ImGui::GetContentRegionAvail().y;
        constexpr float minPanelHeight = 120.0f;
        constexpr float splitterHeight = 8.0f;
        const float maxHierarchyHeight = (std::max)(minPanelHeight, availableHeight - minPanelHeight - splitterHeight);
        m_sceneHierarchyHeight = std::clamp(m_sceneHierarchyHeight, minPanelHeight, maxHierarchyHeight);

        ImGui::BeginChild("SceneHierarchyPanel", ImVec2(0.0f, m_sceneHierarchyHeight), true);
        RenderSceneHierarchy();
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.26f, 0.33f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.36f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.41f, 0.50f, 1.0f));
        ImGui::Button("##ScenePanelSplitter", ImVec2(-1.0f, splitterHeight));
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemActive())
        {
            m_sceneHierarchyHeight += ImGui::GetIO().MouseDelta.y;
            m_sceneHierarchyHeight = std::clamp(m_sceneHierarchyHeight, minPanelHeight, maxHierarchyHeight);
        }

        ImGui::BeginChild("InspectorPanel", ImVec2(0.0f, 0.0f), true);
        RenderInspector();
        ImGui::EndChild();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Input"))
    {
        RenderInputMonitor();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Events"))
    {
        RenderEventMonitor();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Achievements"))
    {
        RenderAchievements();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
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

    if (m_pSceneManager == nullptr)
    {
        return false;
    }

    const auto &scenes = m_pSceneManager->GetScenes();
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

    if (m_pSceneManager == nullptr)
    {
        ImGui::TextDisabled("Scene manager unavailable.");
        return;
    }

    const auto &scenes = m_pSceneManager->GetScenes();
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
    if (m_pInputManager == nullptr)
    {
        ImGui::TextDisabled("Input manager unavailable.");
        return;
    }

    const auto &input = *m_pInputManager;

    ImGui::TextDisabled("Keyboard");
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
    ImGui::Separator();
    ImGui::TextDisabled("Controller");

    if (const auto gamepadState = input.GetGamepadState(InputManager::AnyGamepad))
    {
        const float leftMagnitude = StickMagnitude(gamepadState->leftThumbX, gamepadState->leftThumbY);
        const float rightMagnitude = StickMagnitude(gamepadState->rightThumbX, gamepadState->rightThumbY);
        constexpr float maxStickMagnitude = 32767.0f;

        ImGui::Text("%.*s slot %u", static_cast<int>(gamepadState->backend.size()), gamepadState->backend.data(), gamepadState->playerIndex);
        ImGui::Text("Packet: %u", gamepadState->packetNumber);
        ImGui::ProgressBar((std::min)(1.0f, leftMagnitude / maxStickMagnitude), ImVec2(-1.0f, 0.0f), "Left stick magnitude");
        ImGui::ProgressBar((std::min)(1.0f, rightMagnitude / maxStickMagnitude), ImVec2(-1.0f, 0.0f), "Right stick magnitude");

        if (ImGui::BeginTable("ControllerDetails", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Buttons");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%04X", gamepadState->buttons);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Left stick");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d, %d", gamepadState->leftThumbX, gamepadState->leftThumbY);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Right stick");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d, %d", gamepadState->rightThumbX, gamepadState->rightThumbY);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Triggers");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%u / %u", gamepadState->leftTrigger, gamepadState->rightTrigger);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Deadzones");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("L:%d R:%d", gamepadState->leftDeadzone, gamepadState->rightDeadzone);

            ImGui::EndTable();
        }
    }
    else
    {
        ImGui::TextDisabled("No controller connected.");
    }
}

void dae::ImGuiManager::RenderEventMonitor() const
{
    const auto &subjects = Subject::GetLiveSubjects();
    const auto &notifications = Subject::GetRecentNotifications();

    std::size_t totalObservers{};
    std::size_t playerDiedCount{};
    std::size_t pointsGainedCount{};

    for (const Subject *subject : subjects)
    {
        if (subject != nullptr)
        {
            totalObservers += subject->GetObserverCount();
        }
    }

    for (const auto &record : notifications)
    {
        if (record.event == GameEvent::PlayerDied)
        {
            ++playerDiedCount;
        }
        else if (record.event == GameEvent::PointsGained)
        {
            ++pointsGainedCount;
        }
    }

    ImGui::Text("Live subjects: %zu", subjects.size());
    ImGui::Text("Registered observers: %zu", totalObservers);
    const std::size_t totalEvents = playerDiedCount + pointsGainedCount;
    ImGui::Text("Tracked event types: %zu | Total captured: %zu", static_cast<std::size_t>(2), totalEvents);

    const float totalEventsAsFloat = totalEvents > 0 ? static_cast<float>(totalEvents) : 1.0f;
    const float diedRatio = static_cast<float>(playerDiedCount) / totalEventsAsFloat;
    const float pointsRatio = static_cast<float>(pointsGainedCount) / totalEventsAsFloat;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.77f, 0.36f, 0.36f, 1.0f));
    ImGui::ProgressBar(diedRatio, ImVec2(-1.0f, 0.0f), "PlayerDied");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", playerDiedCount);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.32f, 0.72f, 0.47f, 1.0f));
    ImGui::ProgressBar(pointsRatio, ImVec2(-1.0f, 0.0f), "PointsGained");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", pointsGainedCount);

    ImGui::Separator();

    ImGui::TextDisabled("Subject registry (live runtime state)");
    if (subjects.empty())
    {
        ImGui::TextDisabled("No active subjects.");
    }
    else
    {
        if (ImGui::BeginTable("SubjectTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Subject Address");
            ImGui::TableSetupColumn("Observers");
            ImGui::TableHeadersRow();

            for (std::size_t index{}; index < subjects.size(); ++index)
            {
                const Subject *subject = subjects[index];

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%zu", index);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("0x%llX", static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(subject)));
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%zu", subject != nullptr ? subject->GetObserverCount() : 0u);
            }

            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Recent notifications (newest first)");
    if (notifications.empty())
    {
        ImGui::TextDisabled("No notifications yet.");
        return;
    }

    if (ImGui::BeginChild("EventTimelineRegion", ImVec2(0.0f, 0.0f), true))
    {
        if (ImGui::BeginTable("NotificationTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Seq", ImGuiTableColumnFlags_WidthFixed, 56.0f);
            ImGui::TableSetupColumn("Event", ImGuiTableColumnFlags_WidthFixed, 128.0f);
            ImGui::TableSetupColumn("Subject");
            ImGui::TableSetupColumn("Observers", ImGuiTableColumnFlags_WidthFixed, 88.0f);
            ImGui::TableHeadersRow();

            const std::size_t rowCount = (std::min)(notifications.size(), static_cast<std::size_t>(40));
            for (std::size_t index{}; index < rowCount; ++index)
            {
                const auto &record = notifications[notifications.size() - 1 - index];

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%llu", static_cast<unsigned long long>(record.sequence));

                ImGui::TableSetColumnIndex(1);
                const ImVec4 eventColor =
                    record.event == GameEvent::PlayerDied ? ImVec4(0.88f, 0.47f, 0.47f, 1.0f)
                                                          : ImVec4(0.44f, 0.84f, 0.58f, 1.0f);
                ImGui::TextColored(eventColor, "%s", ToText(record.event));

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("0x%llX", static_cast<unsigned long long>(record.subjectAddress));

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%zu", record.observerCount);
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

void dae::ImGuiManager::RenderAchievements()
{
    Achievements *achievements = Achievements::GetActiveInstance();
    const bool hasInstance = achievements != nullptr;
    const bool steamReady = hasInstance && achievements->IsSteamAvailable();
    const std::uint32_t currentAppId = hasInstance ? achievements->GetCurrentAppId() : 0;
    const bool isSpacewarApp = hasInstance && achievements->IsSpacewarAppIdActive();
    const bool canClearSpacewar = steamReady && isSpacewarApp;

    ImGui::Text("Achievements system: %s", hasInstance ? "active" : "not created");
    ImGui::Text("Steam integration: %s", steamReady ? "ready" : "unavailable");
    ImGui::Text("Current AppID: %u", currentAppId);
    ImGui::Text("Spacewar safety gate: %s", isSpacewarApp ? "AppID 480 confirmed" : "blocked (AppID must be 480)");
    ImGui::Separator();

    ImGui::TextWrapped("This action only targets achievements and only when running Spacewar (AppID 480). Non-480 apps are blocked.");
    ImGui::Checkbox("I confirm clearing Spacewar (480) achievements only", &m_confirmResetAllStats);

    if (!canClearSpacewar)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Clear Spacewar (480) Achievements", ImVec2(-1.0f, 34.0f)))
    {
        if (m_confirmResetAllStats && canClearSpacewar)
        {
            const bool wasReset = achievements->ClearSpacewarAchievementsOnly();
            m_achievementActionFeedback = wasReset ? "Spacewar achievements clear request sent successfully." : "Could not clear achievements. Make sure Steam is running and current app id is 480.";
            if (wasReset)
            {
                m_confirmResetAllStats = false;
            }
        }
        else if (!m_confirmResetAllStats)
        {
            m_achievementActionFeedback = "Enable the confirmation checkbox first.";
        }
        else if (!isSpacewarApp)
        {
            m_achievementActionFeedback = "Blocked. Active AppID is not 480, so only Spacewar-safe reset is allowed.";
        }
    }

    if (!canClearSpacewar)
    {
        ImGui::EndDisabled();
    }

    if (!m_achievementActionFeedback.empty())
    {
        ImGui::Spacing();
        ImGui::TextWrapped("%s", m_achievementActionFeedback.c_str());
    }
}
