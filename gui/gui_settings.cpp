//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_settings.h"
#include "imgui.h"

using namespace Verse;

void Gui::settings(Scene &s, Config &c) {
    ImGui::Begin("Config");
    
    ImGui::Checkbox("Use grayscale", &c.use_grayscale);
    ImGui::Checkbox("Use light", &c.use_light);
    ImGui::SliderInt("Palette", &c.palette_index, -1, c.num_palettes - 1);
    
    ImGui::End();
}
