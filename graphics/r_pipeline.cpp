//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "imgui.h"
#include "gui.h"

#include "system_list.h"

#include "r_pipeline.h"
#include "r_window.h"
#include "r_shader.h"
#include "r_opengl.h"
#include "r_renderer.h"
#include "r_textures.h"

using namespace Verse;
using namespace Graphics;

namespace {
    SDL_Window *window;
    int refresh_rate = 60;
}

void Graphics::init(Config &c) {
    
    //MODERN OPENGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #ifndef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    #endif
    
    //CREATE A WINDOW
    window = Graphics::Window::createWindow(c);
    
    //REFRESH RATE
    Graphics::calculateRefreshRate();
    log::graphics("Refresh Rate: %d", refresh_rate);
    
    //RENDERER
    Renderer::create(c, window);
}


void Graphics::render(Scene &scene, Config &c, ui16 fps) {
    //CLEAR
    glViewport(0, 0, c.resolution.x + 2, c.resolution.y + 2);
    Renderer::clear(scene, c);
    
    //PRERENDER GUI
    if (c.enable_gui)
        Gui::prerender(scene, c, fps, window);
    
    //RENDER SYSTEMS
    RENDER_SYSTEMS

    //RENDER TO WINDOW
    Renderer::renderPost(c);
    glViewport(0, 0, c.resolution.x * c.render_scale, c.resolution.y * c.render_scale);
    Renderer::renderCam(c);
    glViewport(0, 0, c.window_size.x, c.window_size.y);
    Renderer::renderWindow(c);
    
    //RENDER GUI
    if (c.enable_gui)
        Gui::render();
    
    //PRESENT
    Renderer::present(window);
}


void Graphics::destroy() {
    Renderer::destroy();
    SDL_DestroyWindow(window);
}


void Graphics::calculateRefreshRate() {
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    
    SDL_DisplayMode mode;
    SDL_GetDisplayMode(displayIndex, 0, &mode);
    
    refresh_rate = mode.refresh_rate;
}


int Graphics::getRefreshRate() {
    return refresh_rate;
}