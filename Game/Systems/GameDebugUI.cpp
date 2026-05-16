#include "GameDebugUI.h"
#include "ImGuiManager.h"
#include "Achievements.h"
#include "MazeDrawingComponent.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "SoundSystem.h"
#include "AudioLogger.h"
#include <imgui.h>

namespace dae
{
    void RenderAchievementsTab()
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
        
        static bool confirmReset = false;
        ImGui::Checkbox("I confirm clearing Spacewar (480) achievements only", &confirmReset);

        if (!canClearSpacewar)
        {
            ImGui::BeginDisabled();
        }

        static std::string feedback;
        if (ImGui::Button("Clear Spacewar (480) Achievements", ImVec2(-1.0f, 34.0f)))
        {
            if (confirmReset && canClearSpacewar)
            {
                const bool wasReset = achievements->ClearSpacewarAchievementsOnly();
                feedback = wasReset ? "Spacewar achievements clear request sent successfully." : "Could not clear achievements. Make sure Steam is running and current app id is 480.";
                if (wasReset)
                {
                    confirmReset = false;
                }
            }
            else if (!confirmReset)
            {
                feedback = "Enable the confirmation checkbox first.";
            }
        }

        if (!canClearSpacewar)
        {
            ImGui::EndDisabled();
        }

        if (!feedback.empty())
        {
            ImGui::Spacing();
            ImGui::TextWrapped("%s", feedback.c_str());
        }
    }

    void RenderMazeTab()
    {
        ImGui::Checkbox("Show Full Maze (Debug)", &MazeDrawingComponent::g_ShowFullMaze);
        ImGui::Checkbox("Show Collision Grid (Debug)", &CollisionGrid::g_ShowCollisionGrid);
    }

    void GameDebugUI::RegisterCustomTabs(ImGuiManager& imGuiManager)
    {
        imGuiManager.AddCustomTab("Achievements", RenderAchievementsTab);
        imGuiManager.AddCustomTab("Maze", RenderMazeTab);
    }
}
