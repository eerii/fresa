//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_menu.h"

#include "serialization.h"
#include "scene_list.h"

#include "r_window.h"
#include "s_tilemap.h"
#include "s_scene_transition.h"

using namespace Verse;

namespace {
    float mean_physics_time, mean_render_time;
    float physics_accumulator, render_accumulator;
    ui32 time_passed;
    ui16 frames;
}

void Gui::menu(Config &c) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    if(ImGui::BeginMainMenuBar()) {
        //LUME
        //---------------------------
        if (ImGui::BeginMenu("[ lume ]")) {
            ImGui::Text("version: %s", c.version.c_str());
            ImGui::Text("res: %d x %d (x%d)", c.resolution.x, c.resolution.y, c.render_scale);
            
            str scene_name = "scene: " + ((c.active_scene != nullptr) ? c.active_scene->name : "NULL");
            if (ImGui::BeginMenu(scene_name.c_str())) {
#ifdef DEBUG
                //WARNING: Using this feature creates new scenes. Hopefully this should be managed, but it might be prone to errors.
                
                if (ImGui::BeginListBox(""))
                {
                    for (str name : scenes) {
                        const bool is_selected = (name == c.active_scene->name);
                        const char* scene_name = name.c_str();
                        
                        if (ImGui::Selectable(scene_name, is_selected)) {
                            Scene* s = new Scene();
                            Serialization::loadScene(name, s, c);
                            System::SceneTransition::handle(c, s, s->checkpoints[0]);
                        }
                        
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }
#endif
                ImGui::EndMenu();
            }
            
            if (c.active_camera == nullptr)
                ImGui::Text("camera: NULL");
            ImGui::EndMenu();
        }
        //---------------------------
        
        //OPTIONS
        //---------------------------
        if (ImGui::BeginMenu("opcións")) {
            ImGui::Text("game");
            float speed = c.game_speed;
            ImGui::DragFloat("game speed", &speed, 0.1, 0.0, 3.0);
            c.game_speed = speed;
            
            ImGui::Checkbox("collision boxes", &c.render_collision_boxes);
            ImGui::EndMenu();
        }
        //---------------------------
        
        //COLOR
        //---------------------------
        if (ImGui::BeginMenu("color")) {
            ImGui::SliderInt("palette", &c.palette_index, -1, c.num_palettes - 1);
            ImGui::Checkbox("use light", &c.use_light);
            ImGui::Checkbox("use grayscale", &c.use_grayscale);
            ImGui::EndMenu();
        }
        //---------------------------
        
        //CAMERA
        //---------------------------
        if (ImGui::BeginMenu("cámara")) {
            ImGui::Checkbox("subpixel camera", &c.use_subpixel_cam);
            ImGui::Checkbox("enable lookahead", &c.enable_lookahead);
            ImGui::Checkbox("enable smooth panning", &c.enable_smooth_panning);
            if (ImGui::Checkbox("use vsync", &c.use_vsync))
                Graphics::Window::updateVsync(c);
            ImGui::EndMenu();
        }
        //---------------------------
        
        //UTILIDADES
        //---------------------------
        if (ImGui::BeginMenu("utilidades")) {
            ImGui::Checkbox("entities", &ActiveWindows::entities);
            ImGui::Checkbox("test", &ActiveWindows::test);
            
            ImGui::EndMenu();
        }
        //---------------------------
        
        //FPS
        //---------------------------
        str fps_menu = "fps: " + std::to_string(c.fps);
        ImVec2 size = ImGui::CalcTextSize(fps_menu.c_str());
        size.x += style.FramePadding.x * 2 + style.ItemSpacing.x;
        
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x, 0));

        frames++;
        time_passed += Time::delta;
        physics_accumulator += c.physics_time;
        render_accumulator += c.render_time;
        if (time_passed > 200) {
            mean_physics_time = (float)physics_accumulator / (float)(frames);
            mean_render_time = (float)render_accumulator / (float)(frames);
            physics_accumulator = 0;
            render_accumulator = 0;
            time_passed = 0;
            frames = 0;
        }
        
        if (ImGui::BeginMenu(fps_menu.c_str())) {
            
            ImGui::Text("physics time: %f ms", mean_physics_time);
            ImGui::Text("render time: %f ms", mean_render_time);
            
            ImGui::EndMenu();
        }
        //---------------------------
        
        ImGui::EndMainMenuBar();
    }
}
