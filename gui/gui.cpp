//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui.h"

#ifndef DISABLE_GUI

#include "game.h"
#include "input.h"

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "gui_menu.h"
#include "gui_entities.h"
#include "gui_tilemap_editor.h"

using namespace Verse;

struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

bool Gui::ActiveWindows::entities = false;
bool Gui::ActiveWindows::test = false;

void Gui::init(Config &c) {
    ImGuiIO& imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0;
    style.ChildBorderSize = 0;
    style.PopupBorderSize = 0;
    
    style.WindowRounding = 8;
    style.ChildRounding = 8;
    style.PopupRounding = 8;
    style.FrameRounding = 6;
    
    //Colors
    ImVec3 color_for_text = ImVec3(254.f / 255.f, 214.f / 255.f, 173.f / 255.f);
    ImVec3 color_for_head = ImVec3(92.f / 255.f, 133.f / 255.f, 219.f / 255.f);
    ImVec3 color_for_area = ImVec3(42.f / 255.f, 67.f / 255.f, 151.f / 255.f);
    ImVec3 color_for_body = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    ImVec3 color_for_pops = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    style.Colors[ImGuiCol_Text] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 1.00f );
    style.Colors[ImGuiCol_TextDisabled] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.58f );
    style.Colors[ImGuiCol_WindowBg] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 0.95f );
    style.Colors[ImGuiCol_Border] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 0.00f );
    style.Colors[ImGuiCol_BorderShadow] = ImVec4( color_for_body.x, color_for_body.y, color_for_body.z, 0.00f );
    style.Colors[ImGuiCol_FrameBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_TitleBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.75f );
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 0.47f );
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4( color_for_area.x, color_for_area.y, color_for_area.z, 1.00f );
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.21f );
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_CheckMark] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.80f );
    style.Colors[ImGuiCol_SliderGrab] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.50f );
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_Button] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.50f );
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.86f );
    style.Colors[ImGuiCol_ButtonActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_Header] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.76f );
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.86f );
    style.Colors[ImGuiCol_HeaderActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.15f );
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.78f );
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_PlotLines] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.63f );
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4( color_for_text.x, color_for_text.y, color_for_text.z, 0.63f );
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 1.00f );
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( color_for_head.x, color_for_head.y, color_for_head.z, 0.43f );
    style.Colors[ImGuiCol_PopupBg] = ImVec4( color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f );
}

void Gui::update(Config &c) {
    ImGuiIO& imgui_io = ImGui::GetIO();
    
    imgui_io.DeltaTime = c.timestep * c.game_speed;
    
    Vec2f mouse_pos = Vec2f(Input::mouse().x, Input::mouse().y);
    
    imgui_io.MousePos = ImVec2(mouse_pos.x, mouse_pos.y);
    imgui_io.MouseDown[0] = Input::down(SDL_BUTTON_LEFT);
    imgui_io.MouseDown[1] = Input::down(SDL_BUTTON_RIGHT);
    imgui_io.MouseWheel = static_cast<float>(Input::mouseWheel());
    
    std::copy(std::begin(Input::state()->keyboard.down), std::end(Input::state()->keyboard.down), std::begin(imgui_io.KeysDown));
}

void Gui::prerender(Config &c, SDL_Window* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    
    Gui::menu(c);
    
    if (ActiveWindows::test)
        ImGui::ShowDemoWindow();
    
    if (ActiveWindows::entities)
        Gui::entities(c);
}

void Gui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::addInputKey(SDL_Keycode k) {
    str key_name = SDL_GetKeyName(k);
    
    if (key_name == "Space")
        key_name = " ";
    
    if (Input::shift()) {
        if (key_name == "-")
            key_name = "_";
        if (key_name == "1")
            key_name = "!";
        if (key_name == ",")
            key_name = ";";
        if (key_name == ".")
            key_name = ":";
        if (key_name == "+")
            key_name = "*";
        if (key_name == "7")
            key_name = "/";
    }
    
    if (key_name.size() == 1) {
        if (not Input::shift())
            key_name[0] = std::tolower(key_name[0]);
        
        ImGui::GetIO().AddInputCharacter(key_name[0]);
    }
    
    if (key_name.size() == 2 and key_name[0] == 'D') {
        if (not Input::shift())
            ImGui::GetIO().AddInputCharacter(key_name[1]);
    }
}

#endif
