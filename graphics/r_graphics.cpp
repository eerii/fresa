//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_graphics.h"

#include "r_window.h"
#include "r_next_renderer.h"

using namespace Verse;
using namespace Graphics;

namespace {
    WindowData win;
    RenderData render;
}

bool Graphics::init() {
    //Pre configuration
    API::configure();
    
    //Create window
    str version = std::to_string(Conf::version[0]) + "." + std::to_string(Conf::version[1]) + "." + std::to_string(Conf::version[2]);
    str name = Conf::name + " - Version " + version;
    win = Window::create(Conf::window_size, name);
    
    //Create renderer
    render = Renderer::create(win, Conf::resolution);
    
    return true;
}

bool Graphics::update() {
    Renderer::test(win, render);
    
    return true;
}

bool Graphics::stop() {
    return true;
}
