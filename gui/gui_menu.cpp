//project verse, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#ifndef DISABLE_GUI

#include "gui.h"
#include "config.h"

using namespace Fresa;

void Gui::win_menu() {
    if (ImGui::BeginMainMenuBar()) {
        //---Main---
        if (ImGui::BeginMenu("[ fresa ]")) {
            //: Version
            static const str version = std::to_string(Config::version[0]) + "." +
                                       std::to_string(Config::version[1]) + "." +
                                       std::to_string(Config::version[2]);
            ImGui::Text("version: %s", version.c_str());
            
            //: Resolution and scale (todo)
            //: Scene and select scene (todo)
            //: Camera and different projections (todo)
            
            ImGui::EndMenu();
        }
        
        //---Options---
        if (ImGui::BeginMenu("game")) {
            
            ImGui::EndMenu();
        }
        
        //---Options---
        if (ImGui::BeginMenu("utils")) {
            for (auto &win : windows) {
                if (win.name == "menu") continue;
                ImGui::Checkbox(win.name.c_str(), &win.active);
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

#endif
