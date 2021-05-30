//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_pipeline.h"

#include "log.h"
#include "ftime.h"

#include "gui.h"

#include "r_opengl.h"
#include "r_window.h"
#include "r_renderer.h"
#include "system_list.h"

using namespace Verse;
using namespace Graphics;

namespace {
    SDL_Window *window;
    int refresh_rate = 60;
}

void Graphics::init(Config &c) {
    log::debug("Initializing graphics");
    
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    SDL_Renderer *renderer = NULL;
    SDL_CreateWindowAndRenderer(c.window_size.x, c.window_size.y, SDL_WINDOW_OPENGL, &window, &renderer);
#else
    //OPENGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #ifndef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    #endif //__APPLE__
    
    //CREATE A WINDOW
    window = Graphics::Window::createWindow(c);
    if(window == nullptr)
      
        log::error("There was an error with the window pointer, check r_pipeline");
#endif
    
    //REFRESH RATE
    Graphics::calculateRefreshRate();
    log::graphics("Refresh Rate: %d", refresh_rate);
    
    //RENDERER
    Renderer::create(c, window);
}


void Graphics::render(Config &c) {
    ui64 time_before_render = time_precise();
    
    //CLEAR
    glViewport(0, 0, c.resolution.x + 2, c.resolution.y + 2);
    Renderer::clear(c);
    
#ifndef DISABLE_GUI
    //PRERENDER GUI
    if (c.enable_gui)
        Gui::prerender(c, window);
#endif
    
    //RENDER SYSTEMS
    glEnable(GL_DEPTH_TEST);
    RENDER_SYSTEMS
    glDisable(GL_DEPTH_TEST);

    //RENDER TO WINDOW
    Renderer::renderPost(c);
    glViewport(0, 0, c.resolution.x * c.render_scale, c.resolution.y * c.render_scale);
    Renderer::renderCam(c);
    glViewport(0, 0, c.window_size.x, c.window_size.y);
    Renderer::renderWindow(c);
    
#ifndef DISABLE_GUI
    //RENDER GUI
    if (c.enable_gui)
        Gui::render();
#endif
    
    c.render_time = time_precise_difference(time_before_render);
    
    //PRESENT
    Renderer::present(window);
}


void Graphics::destroy() {
    Renderer::destroy();
    if (window != nullptr)
        SDL_DestroyWindow(window);
}


void Graphics::calculateRefreshRate() {
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    
    SDL_DisplayMode mode;
    if(SDL_GetDisplayMode(displayIndex, 0, &mode))
        log::error("Error getting display mode: ", SDL_GetError());
    
    refresh_rate = mode.refresh_rate;
}


int Graphics::getRefreshRate() {
    return refresh_rate;
}
