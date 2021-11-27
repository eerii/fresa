//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_graphics.h"

#include "r_window.h"

#include "ftime.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Verse;
using namespace Graphics;

namespace {
    WindowData win;
    RenderData render;
    GraphicsAPI api;
}

bool Graphics::init() {
    //Pre configuration
    API::configure();
    
    //Create window
    str version = std::to_string(Conf::version[0]) + "." + std::to_string(Conf::version[1]) + "." + std::to_string(Conf::version[2]);
    str name = Conf::name + " - Version " + version;
    win = Window::create(Conf::window_size, name);
    
    //Create renderer api
    api = API::create(win);
    
    //Calculate resolution and scale
    render.resolution = Conf::window_size;
    Vec2<float> ratios = win.size.to<float>() / render.resolution.to<float>();
    render.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //V-Sync
    render.vsync = true;
    
    return true;
}

bool Graphics::update() {
    //---Example update---
    static DrawID test_draw_id = getDrawID_Cube();
    static DrawID test_draw_id_2 = getDrawID_Cube();
    static TextureData test_texture_data{};
    
    //: Uniforms
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    //Draw something for test (This would be called outside of the renderer)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.8f * std::sin(t * 1.570796f)));
    model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
    model = glm::rotate(model, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    API::draw(test_texture_data, test_draw_id, model);
    
    glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.8f * std::sin(t * 1.570796f + 1.570796f)));
    model2 = glm::scale(model2, glm::vec3(0.4f, 0.4f, 0.4f));
    model2 = glm::rotate(model2, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    API::draw(test_texture_data, test_draw_id_2, model2);
    
    //: Render
    API::renderTest(api, win, render);
    
    return true;
}

bool Graphics::stop() {
    API::clean(api);
    
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
    API::resize(api, win);
}

DrawID Graphics::getDrawID(const std::vector<VertexData> &vertices, const std::vector<ui16> &indices) {
    DrawBufferID buffer = API::registerDrawBuffer(api, vertices, indices);
    return API::registerDrawData(api, buffer);
}

DrawID Graphics::getDrawID_Cube() {
    static DrawBufferID cube_buffer = API::registerDrawBuffer(api, VerticesDefinitions::cube_vertices, VerticesDefinitions::cube_indices);
    return API::registerDrawData(api, cube_buffer);
}

DrawID Graphics::getDrawID_Rect() {
    static DrawBufferID rect_buffer = API::registerDrawBuffer(api, VerticesDefinitions::rect_vertices, VerticesDefinitions::rect_indices);
    return API::registerDrawData(api, rect_buffer);
}
