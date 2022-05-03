//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#ifndef DISABLE_GUI

#include "gui.h"
#include "config.h"
#include "scene.h"
#include "r_graphics.h"

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
            
            ImGui::Text("scene: %s", scene_list.at(active_scene).name.c_str());
            
            ImGui::EndMenu();
        }
        
        //---Options---
        if (ImGui::BeginMenu("game")) {
            ImGui::SetNextItemWidth(100.0f);
            slider("game speed", Config::game_speed, 0.1f, 0.0f, 3.0f);
            
            ImGui::SetNextItemWidth(100.0f);
            if (slider<ui8>("multisampling", Config::multisampling, 1.0f, 0, 6)) Graphics::resize(); //TODO: Propper recreation of the pipeline
            
            ImGui::EndMenu();
        }
        
        //---Camera---
        if (ImGui::BeginMenu("camera")) {
            if (Graphics::camera.projection & Graphics::PROJECTION_SCALED)
                ImGui::Text("res: %dx%d", Config::resolution.x, Config::resolution.y);
            else
                ImGui::Text("res: %dx%d", Graphics::window.size.x, Graphics::window.size.y);
            
            if (Graphics::camera.projection & Graphics::PROJECTION_PERSPECTIVE)
                ImGui::Text("proj: perspective");
            if (Graphics::camera.projection & Graphics::PROJECTION_ORTHOGRAPHIC)
                ImGui::Text("proj: orthographic");
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("pos:");
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.x, 1.0f, -10000.0f, 10000.0f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.y, 1.0f, -10000.0f, 10000.0f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.z, 1.0f, -10000.0f, 10000.0f);
            
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("dir:");
            ImGui::SameLine(); ImGui::SetNextItemWidth(70.0f);
            slider("", Graphics::camera.phi, 0.01f, -0.0f, 6.28f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(70.0f);
            slider("", Graphics::camera.theta, 0.01f, -0.0f, 3.14f);
            
            ImGui::EndMenu();
        }
        
        
        //---Utils---
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
