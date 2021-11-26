//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_graphics.h"

#include "r_window.h"

#include "ftime.h"

using namespace Verse;
using namespace Graphics;

namespace {
    WindowData win;
    RenderData render; //TODO: MOVE API OUT OF HERE
}

bool Graphics::init() {
    //Pre configuration
    API::configure();
    
    //Create window
    str version = std::to_string(Conf::version[0]) + "." + std::to_string(Conf::version[1]) + "." + std::to_string(Conf::version[2]);
    str name = Conf::name + " - Version " + version;
    win = Window::create(Conf::window_size, name);
    
    //Create renderer api
    render.api = API::create(win);
    
    //Calculate resolution and scale
    render.resolution = Conf::window_size;
    Vec2<float> ratios = win.size.to<float>() / render.resolution.to<float>();
    render.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //V-Sync
    render.vsync = true;
    
    return true;
}

bool Graphics::update() {
    API::renderTest(win, render);
    
    return true;
}

bool Graphics::stop() {
    API::clean(render.api);
    
    return true;
}

void Graphics::onResize(Vec2<> size) {
    //New window size
    win.size = size;
    
    //Adjust render scale
    Vec2<float> ratios = win.size.to<float>() / render.resolution.to<float>();
    render.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //GUI adjustment
    #ifndef DISABLE_GUI
    ImGuiIO& io = ImGui::GetIO(); //TODO: Save this somewhere where we can access it later
    io.DisplaySize.x = static_cast<float>(size.x);
    io.DisplaySize.y = static_cast<float>(size.y);
    #endif
    
    //Pass the resize command to the API
    API::resize(render.api, win);
}
