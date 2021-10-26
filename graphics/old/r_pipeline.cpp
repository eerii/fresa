//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_pipeline.h"

#include "log.h"
#include "ftime.h"

#include "gui.h"

#include "r_opengl_core.h"
#include "r_vulkan_core.h"
#include "r_window.h"
#include "r_renderer.h"
#include "system_list.h"

using namespace Verse;
using namespace Graphics;

bool Graphics::init(Config &c) {
    log::debug("Initializing graphics");
    
    //OpenGL Configuration
#ifdef USE_OPENGL
    #ifdef __EMSCRIPTEN__
        GL::configWebGL();
    #else
        GL::configOpenGL();
    #endif
#endif
 
    //Window Creation
    c.window = Window::createWindow(c);
    if(c.window == nullptr) {
        log::error("There was an error with the window pointer, check r_pipeline");
        return false;
    }
    
    //Refresh Rate
    Window::calculateRefreshRate(c);
    log::graphics("Refresh Rate: %d", c.refresh_rate);
    
    //Renderer Creation
    Renderer::create(c);
    
    //GUI
#ifndef DISABLE_GUI
    Gui::init(c);
#endif
    
    return true;
}


void Graphics::render(Config &c) {
    Clock::time_point time_before_render = time();
    
    //CLEAR
    Renderer::clear(c);
    
#ifndef DISABLE_GUI
    //PRERENDER GUI
    Gui::prerender(c, c.window);
#endif
    
    //RENDER SYSTEMS
    Renderer::toggleDepthTest(true);
    System::renderUpdateSystems(c);
    Renderer::toggleDepthTest(false);

    //RENDER TO WINDOW
    if (c.use_light or c.show_light)
        Renderer::renderLight(c);
    Renderer::renderPost(c);
    
    if(c.render_collision_boxes)
        System::Collider::render(c);
    
#ifndef DISABLE_GUI
    if(c.tme_active)
        System::Tilemap::renderEditor(c);
#endif
    
    Renderer::renderCam(c);
    
    Renderer::renderWindow(c);
    
#ifndef DISABLE_GUI
    //RENDER GUI
    Gui::render();
#endif
    
#ifdef USE_VULKAN
    Renderer::renderTest(c);
#endif
    
    c.render_time = ns(time() - time_before_render);
    
    //PRESENT
    Renderer::present(c.window);
}


void Graphics::destroy() {
    Renderer::destroy();
}
