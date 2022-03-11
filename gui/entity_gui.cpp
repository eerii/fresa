//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#ifndef DISABLE_GUI

#include "gui.h"
#include "scene.h"

using namespace Fresa;

void Gui::win_entities() {
    ImGui::Begin("entities");
    
    if (not scene_list.count(active_scene)) {
        ImGui::Text("no active scene");
    } else {
        Scene &scene = scene_list.at(active_scene);
        for (EntityID e : SceneView<>(scene)) {
            ImGui::Text("%s", scene.getName(e).c_str());
        }
    }
    
    ImGui::End();
}

#endif
