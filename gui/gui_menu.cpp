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
            slider("game speed", Config::game_speed, 0.1f, 0.0f, 3.0f);
            
            if (slider<ui8>("multisampling", Config::multisampling, 1.0f, 0, 6)) Graphics::API::resize(Graphics::api, Graphics::win);
            ImGui::Checkbox("draw indirect", &Config::draw_indirect);
            
            ImGui::EndMenu();
        }
        
        //---Camera---
        if (ImGui::BeginMenu("camera")) {
            if (Graphics::camera.proj_type & Graphics::PROJECTION_SCALED)
                ImGui::Text("res: %dx%d (x%d)", Config::resolution.x, Config::resolution.y, Graphics::win.scale);
            else
                ImGui::Text("res: %dx%d", Graphics::win.size.x, Graphics::win.size.y);
            
            if (Graphics::camera.proj_type & Graphics::PROJECTION_PERSPECTIVE)
                ImGui::Text("proj: perspective");
            if (Graphics::camera.proj_type & Graphics::PROJECTION_ORTHOGRAPHIC)
                ImGui::Text("proj: orthographic");
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("pos:");
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.x, 1.0f, -10000.0f, 10000.0f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.y, 1.0f, -10000.0f, 10000.0f);
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            slider("", Graphics::camera.pos.z, 1.0f, -10000.0f, 10000.0f);
            
            auto normalize_direction = [&](){
                Graphics::camera.direction = glm::normalize(Graphics::camera.direction);
            };
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("dir:");
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            if (slider("", Graphics::camera.direction.x, 0.01f, -1.0f, 1.0f)) normalize_direction();
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            if (slider("", Graphics::camera.direction.y, 0.01f, -1.0f, 1.0f)) normalize_direction();
            ImGui::SameLine(); ImGui::SetNextItemWidth(50.0f);
            if (slider("", Graphics::camera.direction.z, 0.01f, -1.0f, 1.0f)) normalize_direction();
            
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
