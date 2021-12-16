//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_window.h"

#include "log.h"
#include "gui.h"

#include "r_vulkan.h"
#include "r_opengl.h"

//: SDL Window Flags
#if defined USE_VULKAN
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    #define RENDERER_NAME "Vulkan"
#elif defined USE_OPENGL
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    #define RENDERER_NAME "OpenGL"
#endif

using namespace Fresa;
using namespace Graphics;

WindowData Graphics::Window::create(Vec2<> size, str name) {
    //---Create window data---
    WindowData win;
    
    //: SDL window
#ifdef __EMSCRIPTEN__
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(size.x, size.y, SDL_WINDOW_OPENGL, &win.window, &renderer);
    
    if (win.window == nullptr or renderer == nullptr)
        log::error("Failed to create a Window and a Renderer", SDL_GetError());
#else
    win.window = SDL_CreateWindow((name + " - " + RENDERER_NAME).c_str(),
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, W_FLAGS);
    
    if (win.window == nullptr)
        log::error("Failed to create a Window", SDL_GetError());
    
    SDL_SetWindowResizable(win.window, SDL_TRUE);
    SDL_SetWindowMinimumSize(win.window, 256, 180);
#endif
    
    //: Window size
    win.size = size;
    
    //: Refresh rate
    win.refresh_rate = getRefreshRate(win);
    log::graphics("Refresh Rate: %d", win.refresh_rate);
    
    //: Calculate resolution and scale
    win.resolution = Config::window_size;
    Vec2<float> ratios = win.size.to<float>() / win.resolution.to<float>();
    win.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //: V-Sync (it only changes something for OpenGL at the moment)
    win.vsync = true;
    
    //: TODO: Projection
    win.proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
    win.proj[1][1] *= -1;
    
    return win;
}

ui16 Graphics::Window::getRefreshRate(WindowData &win, bool force) {
    //---Refresh rate---
    //      Either returns the already calculated refresh rate or it gets it using SDL_DisplayMode
    if (win.refresh_rate > 0 and not force)
        return win.refresh_rate;
    
    int displayIndex = SDL_GetWindowDisplayIndex(win.window);
    
    SDL_DisplayMode mode;
    if(SDL_GetDisplayMode(displayIndex, 0, &mode))
        log::error("Error getting display mode: ", SDL_GetError());
    
    return (ui16)mode.refresh_rate;
}
