//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_window.h"

#include "log.h"
#include "gui.h"
#include "r_renderer.h"

#define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL

using namespace Verse;

SDL_Window* Graphics::Window::createWindow(Config &c) {
    SDL_Window* window = SDL_CreateWindow((c.name + " - Version " + c.version).c_str(),
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              c.window_size.x, c.window_size.y, W_FLAGS);
    if (window == nullptr) {
        log::error("Failed to create a Window", SDL_GetError());
    }
    #if _WIN32
    {
        int display = SDL_GetWindowDisplayIndex(window);
        float ddpi, hdpi, vdpi;
        if (SDL_GetDisplayDPI(display, &ddpi, &hdpi, &vdpi) == 0)
        {
            float hidpi_res = 96;
            float dpi = (ddpi / hidpi_res);
            if (dpi != 1)
            {
                SDL_DisplayMode mode;
                SDL_GetDesktopDisplayMode(display, &mode);
                SDL_SetWindowPosition(window, (int)(mode.w - config->width * dpi) / 2, (int)(mode.h - config->height * dpi) / 2);
                SDL_SetWindowSize(window, (int)(config->width * dpi), (int)(config->height * dpi));
            }
        }
    }
    #endif
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, 256, 180);
    
    return window;
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
}

void Graphics::Window::updateVsync(Config &c) {
#ifndef __EMSCRIPTEN__
    if (SDL_GL_SetSwapInterval(c.use_vsync ? 1 : 0)) {
        str text = c.use_vsync ? "Error enabling V-Sync: " : "Error disabling V-Sync: ";
        log::error(text, SDL_GetError());
    }
#endif
}

Vec2 Graphics::Window::windowToScene(Config &c, Vec2f w_pos) {
    Vec2 pixel_move = Vec2(floor(0.5f * c.resolution.x - c.active_camera->pos.x), floor(0.5f * c.resolution.y - c.active_camera->pos.y));
    
    Vec2 s_pos = w_pos.to_int();
    
    s_pos -= (c.window_size - c.resolution * c.render_scale) * 0.5f;
    s_pos /= (float)c.render_scale;
    s_pos -= pixel_move - Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH);
    
    return s_pos;
}



Vec2f Graphics::Window::sceneToWindow(Config &c, Vec2 s_pos) {
    Vec2 pixel_move = Vec2(floor(0.5f * c.resolution.x - c.active_camera->pos.x), floor(0.5f * c.resolution.y - c.active_camera->pos.y));
    
    Vec2f w_pos = (s_pos + pixel_move - Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH)).to_float();
    
    w_pos *= c.render_scale;
    w_pos += (c.window_size.to_float() - c.resolution.to_float() * c.render_scale) * 0.5f;
    
    return w_pos;
}
