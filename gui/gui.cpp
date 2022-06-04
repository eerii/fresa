//: fresa by jose pazos perez, licensed under GPLv3
#ifndef DISABLE_GUI

#include "gui.h"
#include "config.h"
#include "file.h"
#include "input.h"
#include "r_graphics.h"

using namespace Fresa;

void Gui::init() {
    //---Initialization---
    ImGui::CreateContext();
    
    //---IO---
    io = &ImGui::GetIO();
    
    //: Docking
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    //: Multi viewports
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    //---Backends---
    #if defined USE_OPENGL
    if (not ImGui_ImplSDL2_InitForOpenGL(Graphics::window.window, Graphics::api.context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    #elif defined USE_VULKAN
    Graphics::VK::Gui::init();
    #endif
    
    //---Fonts---
    auto font_path = File::path_optional("misc/font.ttf");
    if (font_path.has_value()) {
        float font_scale = Graphics::window.dpi;
        io->Fonts->AddFontFromFileTTF(font_path.value().c_str(), 16 * font_scale);
        io->FontGlobalScale /= font_scale;
        io->Fonts->Build();
    }
    
    #ifdef USE_VULKAN
    Graphics::VK::Gui::transferFonts();
    #endif
    
    //---Style---
    style = &ImGui::GetStyle();
    
    //: Borders
    style->WindowBorderSize = 0;
    style->ChildBorderSize = 0;
    style->PopupBorderSize = 0;
    
    //: Rounding
    style->WindowRounding = 8;
    style->ChildRounding = 8;
    style->PopupRounding = 8;
    style->FrameRounding = 6;
    
    //: Colors
    ImVec3 color_for_text = ImVec3(254.f / 255.f, 214.f / 255.f, 173.f / 255.f);
    ImVec3 color_for_head = ImVec3(92.f / 255.f, 133.f / 255.f, 219.f / 255.f);
    ImVec3 color_for_area = ImVec3(42.f / 255.f, 67.f / 255.f, 151.f / 255.f);
    ImVec3 color_for_body = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    ImVec3 color_for_pops = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    setColors(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
    
    //---Resize---
    Graphics::event_window_resize.callback([](const Vec2<ui16> &size){
        io->DisplaySize.x = (float)size.x;
        io->DisplaySize.y = (float)size.y;
    });
    
    //---Windows---
    registerWindows();
}

void Gui::GuiSystem::update() {
    //: Delta
    io->DeltaTime = Config::timestep * Config::game_speed;
    
    //: Keyboard and mouse
    Input::gui_using_mouse = io->WantCaptureMouse;
    Input::gui_using_keyboard = io->WantCaptureKeyboard;
}

void Gui::GuiSystem::render() {
    #if defined USE_OPENGL
    ImGui_ImplOpenGL3_NewFrame();
    #elif defined USE_VULKAN
    ImGui_ImplVulkan_NewFrame();
    #endif

    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    widget_id = 0;
    for (auto &win : windows) {
        if (win.active)
            win.show();
    }
    
    ImGui::Render();
    
    //: Multi viewports
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void Gui::setColors(ImVec3 text, ImVec3 head, ImVec3 area, ImVec3 body, ImVec3 pops) {
    style->Colors[ImGuiCol_Text] = ImVec4( text.x, text.y, text.z, 1.00f );
    style->Colors[ImGuiCol_TextDisabled] = ImVec4( text.x, text.y, text.z, 0.58f );
    style->Colors[ImGuiCol_WindowBg] = ImVec4( body.x, body.y, body.z, 0.95f );
    style->Colors[ImGuiCol_Border] = ImVec4( body.x, body.y, body.z, 0.00f );
    style->Colors[ImGuiCol_BorderShadow] = ImVec4( body.x, body.y, body.z, 0.00f );
    style->Colors[ImGuiCol_FrameBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_TitleBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( area.x, area.y, area.z, 0.75f );
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4( area.x, area.y, area.z, 0.47f );
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4( head.x, head.y, head.z, 0.21f );
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_CheckMark] = ImVec4( head.x, head.y, head.z, 0.80f );
    style->Colors[ImGuiCol_SliderGrab] = ImVec4( head.x, head.y, head.z, 0.50f );
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_Button] = ImVec4( head.x, head.y, head.z, 0.50f );
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4( head.x, head.y, head.z, 0.86f );
    style->Colors[ImGuiCol_ButtonActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_Header] = ImVec4( head.x, head.y, head.z, 0.76f );
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4( head.x, head.y, head.z, 0.86f );
    style->Colors[ImGuiCol_HeaderActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4( head.x, head.y, head.z, 0.15f );
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_PlotLines] = ImVec4( text.x, text.y, text.z, 0.63f );
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4( text.x, text.y, text.z, 0.63f );
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4( head.x, head.y, head.z, 0.43f );
    style->Colors[ImGuiCol_PopupBg] = ImVec4( pops.x, pops.y, pops.z, 0.92f );
}

#endif
