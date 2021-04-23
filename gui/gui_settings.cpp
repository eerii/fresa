//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_settings.h"
#include "imgui.h"

using namespace Verse;

void Gui::settings(Config &c) {
    ImGui::Begin("Config");
    
    ImGui::Checkbox("Use dithering", &c.use_dithering);
    ImGui::Checkbox("Use grayscale", &c.use_grayscale);
    ImGui::SliderInt("Palette", &c.palette_index, 0, c.num_palettes - 1);
    
    ImGui::End();
}
