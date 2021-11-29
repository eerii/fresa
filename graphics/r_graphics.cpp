//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_graphics.h"

#include "r_window.h"

#include "ftime.h"
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace Verse;
using namespace Graphics;

namespace {
    WindowData win;
    RenderData render;
    GraphicsAPI api;

    std::map<str, TextureID> texture_locations{};
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
    //This part would not go here
    static TextureID test_texture_data = getTextureID("res/graphics/texture.png");
    
    static DrawID test_draw_id = getDrawID_Rect();
    static DrawID test_draw_id_2 = getDrawID_Cube();
    static DrawID test_draw_id_3 = getDrawID_Cube();
    const std::vector<VertexData> rect_vertices_2 = {
        {{-1.f, -1.f, 0.f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.f, -1.f, 0.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.f, 1.f, 0.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{-1.f, 1.f, 0.f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    };
    static DrawID test_draw_id_4 = getDrawID(rect_vertices_2, VerticesDefinitions::rect_indices);
    static std::array<DrawID, 10> test_draw_ids;
    
    static bool binded = false;
    if (not binded) {
        bindTexture(test_draw_id, test_texture_data);
        bindTexture(test_draw_id_2, test_texture_data);
        bindTexture(test_draw_id_3, test_texture_data);
        bindTexture(test_draw_id_4, test_texture_data);
        
        bool odd = true;
        for (auto &id : test_draw_ids) {
            id = odd ? getDrawID_Cube() : getDrawID(rect_vertices_2, VerticesDefinitions::rect_indices);
            bindTexture(id, test_texture_data);
            odd = not odd;
        }
        
        binded = true;
    }
    
    //: Uniforms
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    //Draw something for test (This would be called outside of the renderer)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.8f * std::sin(t * 1.570796f)));
    model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
    model = glm::rotate(model, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    draw(test_draw_id, model);
    
    glm::mat4 model3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
    double s = 0.1 * std::sin(t * 0.25f * 1.570796) + 0.2;
    model3 = glm::scale(model3, glm::vec3(s, s, s));
    model3 = glm::rotate(model3, t, glm::vec3(0.1f, 0.5f, 0.1f));
    draw(test_draw_id_3, model3);
    
    glm::mat4 model4 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.2f * std::sin(t * 3.141592f)));
    model4 = glm::scale(model4, glm::vec3(0.3f, 0.3f, 0.3f));
    model4 = glm::rotate(model4, 3.14f * std::sin(t), glm::vec3(0.0f, 0.0f, 1.0f));
    draw(test_draw_id_4, model4);
    
    int i = 0;
    for (auto &id : test_draw_ids) {
        float a = i * ((2.0f * 3.141592f) / 10.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(std::sin(a + t*0.1f), std::cos(a + t*0.1f), 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, t * 1.570796f + i * a, glm::vec3(0.0f, 0.0f, 1.0f));
        draw(id, model);
        i++;
    }
    
    glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.8f * std::sin(t * 1.570796f + 1.570796f)));
    model2 = glm::scale(model2, glm::vec3(0.6f, 0.6f, 0.6f));
    model2 = glm::rotate(model2, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    draw(test_draw_id_2, model2);
    
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

TextureID Graphics::getTextureID(str path, Channels ch) {
    //: Check if path exists
    if (not std::filesystem::exists(std::filesystem::path{path}))
        log::error("The texture path does not exist!");
    
    //: Check if texture is already registered
    auto it = texture_locations.find(path);
    if (it != texture_locations.end()) //Exists
        return it->second;

    //---Load the image---
    auto mode = [ch](){
        switch(ch) {
            case TEXTURE_CHANNELS_G:
                return STBI_grey;
            case TEXTURE_CHANNELS_GA:
                return STBI_grey_alpha;
            case TEXTURE_CHANNELS_RGB:
                return STBI_rgb;
            case TEXTURE_CHANNELS_RGBA:
                return STBI_rgb_alpha;
        }
    }();
    
    Vec2<> size;
    int real_ch;
    ui8* pixels = stbi_load(path.c_str(), &size.x, &size.y, &real_ch, mode);
    
    //---Create texture---
    TextureID tex_id = API::registerTexture(api, size, ch, pixels);
    texture_locations[path] = tex_id;
    
    //: Free from memory
    stbi_image_free(pixels);
    
    return tex_id;
}

void Graphics::bindTexture(DrawID draw_id, TextureID tex_id) {
    API::draw_data.at(draw_id).texture_id = tex_id;
    API::updateDescriptorSets(api, &API::draw_data.at(draw_id));
}

void Graphics::unbindTexture(DrawID draw_id) {
    API::draw_data.at(draw_id).texture_id = std::nullopt;
    API::updateDescriptorSets(api, &API::draw_data.at(draw_id));
}

void Graphics::draw(const DrawID draw_id, glm::mat4 model) {
    DrawQueueInfo queue_info{};
    
    queue_info.data = &API::draw_data.at(draw_id);
    if (queue_info.data == nullptr)
        log::error("Tried to draw an object with a null pointer draw data");
    
    queue_info.buffer = &API::draw_buffer_data.at(queue_info.data->buffer_id);
    if (queue_info.buffer == nullptr)
        log::error("Tried to draw an object with a null pointer draw buffer");
    
    queue_info.model = model;
    
    if (queue_info.data->texture_id.has_value()) {
        const TextureData* tex = &API::texture_data.at(queue_info.data->texture_id.value());
        if (tex == nullptr)
            log::error("Tried to draw an object with an invalid texture id");
        API::draw_queue_textures[tex].push_back(queue_info);
        return;
    }
    
    log::warn("The queue for non textured objects is not implemented yet"); //TODO: .
}
