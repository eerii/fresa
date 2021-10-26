//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_pipeline.h"

#include "log.h"
#include "ftime.h"

#include "gui.h"

#include "r_api.h"

#include "r_window.h"
#include "r_renderer.h"
#include "system_list.h"

using namespace Verse;
using namespace Graphics;

namespace {
    WindowData win;
    GraphicsAPI api;
}

bool Graphics::init(Config &c) {
    log::debug("Initializing graphics");
    
    //Pre configuration
    API::configure();
 
    //Window creation
    str version = std::to_string(Conf::version[0]) + "." + std::to_string(Conf::version[1]) + "." + std::to_string(Conf::version[2]);
    str name = Conf::name + " - Version " + version;
    win = Window::create(Conf::window_size.x, Conf::window_size.y, name);
    
    //TODO: REMOVE THIS
    c.window = win.window;
    c.window_size = win.size;
    c.refresh_rate = win.refresh_rate;
    
    //Initialize graphics API
    API::init(&api, win);
    
    //Renderer Creation
    Renderer::create(&api, c);
    
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
    Renderer::renderTest(win);
#endif
    
    c.render_time = ns(time() - time_before_render);
    
    //PRESENT
    Renderer::present(c.window);
}


void Graphics::destroy() {
    Renderer::destroy();
}
