//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_window.h"

#include "log.h"
#include "gui.h"

#include "r_vulkan.h"
#include "r_opengl.h"
#include "r_renderer.h"

#if defined USE_VULKAN
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    #define RENDERER_NAME "Vulkan"
#elif defined USE_OPENGL
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    #define RENDERER_NAME "OpenGL"
#endif

using namespace Verse;
using namespace Graphics;

WindowData Graphics::Window::create(Vec2<> size, str name) {
    WindowData win;
    
    //Create SDL window
#ifdef __EMSCRIPTEN__
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(size_x, size_y, SDL_WINDOW_OPENGL, &win_info.window, &renderer);
    
    if (win_info.window == nullptr or renderer == nullptr)
        log::error("Failed to create a Window and a Renderer", SDL_GetError());
#else
    win.window = SDL_CreateWindow((name + " - " + RENDERER_NAME).c_str(),
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, W_FLAGS);
    
    if (win.window == nullptr)
        log::error("Failed to create a Window", SDL_GetError());
    
    SDL_SetWindowResizable(win.window, SDL_TRUE);
    SDL_SetWindowMinimumSize(win.window, 256, 180);
#endif
    
    //Window size
    win.size = size;
    
    //Refresh rate
    win.refresh_rate = getRefreshRate(win);
    log::graphics("Refresh Rate: %d", win.refresh_rate);
    
    return win;
}

ui16 Graphics::Window::getRefreshRate(WindowData &win) {
    int displayIndex = SDL_GetWindowDisplayIndex(win.window);
    
    SDL_DisplayMode mode;
    if(SDL_GetDisplayMode(displayIndex, 0, &mode))
        log::error("Error getting display mode: ", SDL_GetError());
    
    return (ui16)mode.refresh_rate;
}

void Graphics::Window::onResize(SDL_Event &e, Config &c) {
    bool longer_x = ((float)e.window.data1 / (float)e.window.data2) >= ((float)c.resolution.x / (float)c.resolution.y);
    c.render_scale = longer_x ? floor((float)e.window.data2 / (float)c.resolution.y) : floor((float)e.window.data1 / (float)c.resolution.x);
    
    c.window_size = Vec2(e.window.data1, e.window.data2);
    
#ifndef DISABLE_GUI
    ImGuiIO& imgui_io = ImGui::GetIO();
    imgui_io.DisplaySize.x = static_cast<float>(e.window.data1);
    imgui_io.DisplaySize.y = static_cast<float>(e.window.data2);
#endif
    
    //TODO: Rework
    //Graphics::Renderer::onResize(win);
}

Vec2<> Graphics::Window::windowToScene(Config &c, Vec2<float> w_pos) {
    Vec2 pixel_move = Vec2(floor(0.5f * c.resolution.x - c.active_camera->pos.x), floor(0.5f * c.resolution.y - c.active_camera->pos.y));
    
    Vec2 s_pos = w_pos.to<int>();
    
    s_pos -= (c.window_size - c.resolution * c.render_scale) * 0.5f;
    s_pos /= (float)c.render_scale;
    s_pos -= pixel_move - Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH);
    
    return s_pos;
}

Vec2<float> Graphics::Window::sceneToWindow(Config &c, Vec2<> s_pos) {
    Vec2<> pixel_move = Vec2<>(floor(0.5f * c.resolution.x - c.active_camera->pos.x), floor(0.5f * c.resolution.y - c.active_camera->pos.y));
    
    Vec2<float> w_pos = (s_pos + pixel_move - Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH)).to<float>();
    
    w_pos *= c.render_scale;
    w_pos += (c.window_size.to<float>() - c.resolution.to<float>() * c.render_scale) * 0.5f;
    
    return w_pos;
}
