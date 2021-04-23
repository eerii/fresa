//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_performance.h"
#include "imgui.h"

using namespace Verse;

void Gui::performance(ui16 &fps) {
    ImGui::Begin("Performance");
    
    ImGui::Text("FPS: %d", fps);
    
    ImGui::End();
}
