//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui.h"
#include "input.h"

#include "gui_entities.h"
#include "gui_actors.h"
#include "gui_performance.h"
#include "gui_settings.h"

using namespace Verse;

void Gui::update(float delta, Config &c) {
    ImGuiIO& imgui_io = ImGui::GetIO();
    
    imgui_io.DeltaTime = delta;
    
    Vec2f mouse_pos = Vec2f(Input::mouse().x, Input::mouse().y);
    
    imgui_io.MousePos = ImVec2(mouse_pos.x, mouse_pos.y);
    imgui_io.MouseDown[0] = Input::down(SDL_BUTTON_LEFT);
    imgui_io.MouseDown[1] = Input::down(SDL_BUTTON_RIGHT);
    imgui_io.MouseWheel = static_cast<float>(Input::mouseWheel());
}

void Gui::prerender(Scene &scene, Config &c, ui16 &fps, SDL_Window* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    
    Gui::entities(scene);
    Gui::actors(scene);
    Gui::performance(fps);
    Gui::settings(c);
}

void Gui::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
