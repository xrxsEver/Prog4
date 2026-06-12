#include "GameDebugUI.h"
#include "ImGuiManager.h"
#include "MazeDrawingComponent.h"
#include "ServiceLocator.h"
#include "CollisionGrid.h"
#include "SoundSystem.h"
#include "AudioLogger.h"
#include <imgui.h>

namespace dae
{
    void RenderMazeTab()
    {
        ImGui::Checkbox("Show Full Maze (Debug)", &MazeDrawingComponent::g_ShowFullMaze);
        ImGui::Checkbox("Show Collision Grid (Debug)", &CollisionGrid::g_ShowCollisionGrid);
    }

    void GameDebugUI::RegisterCustomTabs(ImGuiManager& imGuiManager)
    {
        imGuiManager.AddCustomTab("Maze", RenderMazeTab);
    }
}
