#pragma once
#include <functional>

namespace dae
{
    class ImGuiManager;
    
    class GameDebugUI
    {
    public:
        static void RegisterCustomTabs(ImGuiManager& imGuiManager);
    };
}
