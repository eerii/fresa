//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_graphics.h"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "config.h"
#include "ecs.h"
#include "gui.h"
#include "f_time.h"

using namespace Fresa;
using namespace Graphics;

namespace {
    std::map<str, TextureID> texture_locations{};
}

bool Graphics::init() {
    //---Initialization---
    
    //: Pre configuration
    API::configureAPI();
    
    //: Create window
    str version = std::to_string(Config::version[0]) + "." + std::to_string(Config::version[1]) + "." + std::to_string(Config::version[2]);
    str name = Config::name + " - Version " + version;
    win = API::createWindow(Config::window_size, name);
    
    //: Create renderer api
    API::createShaderList();
    API::renderer_description_path = "res/render/renderer_description";
    api = API::createAPI(win);

    //: Set projection
    camera.proj_type = Projection(PROJECTION_ORTHOGRAPHIC | PROJECTION_SCALED);
    updateCameraProjection(camera);
    win.scaled_ubo = (camera.proj_type & PROJECTION_SCALED) ? API::getScaledWindowUBO(win) :
                                                              UniformBufferObject{glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f)};
    
    //: Initialize GUI
    IF_GUI(Gui::init(api, win));
    
    return true;
}

bool Graphics::update() {
    //---Update---
    
    //: Systems
    Performance::render_system_time.clear();
    for (auto &[priority, system] : System::render_update_systems) {
        Performance::render_system_time.push_back(0);
        TIME(Performance::render_system_time.back(), system.second);
    }
    
    //: Update camera view
    updateCameraView(camera);
    
    //: Render
    API::render(api, win, camera);
    
    //: Present
    API::present(api, win);
    
    return true;
}

bool Graphics::stop() {
    //---Cleanup---
    
    API::clean(api);
    return true;
}

void Graphics::onResize(Vec2<> size) {
    //---On resize callback---
    
    //: Set new window size
    win.size = size;
    
    //: Adjust render scale
    Vec2<float> ratios = win.size.to<float>() / Config::resolution.to<float>();
    win.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    if (camera.proj_type & PROJECTION_SCALED)
        win.scaled_ubo = API::getScaledWindowUBO(win);
    updateCameraProjection(camera);
    
    //: Pass the resize command to the API
    API::resize(api, win);
}

TextureID Graphics::getTextureID(str path, Channels ch) {
    //---Create TextureID---
    
    //: Check if path exists
    if (not std::filesystem::exists(std::filesystem::path{path}))
        log::error("The texture path does not exist!");
    
    //: Check if texture is already registered
    auto it = texture_locations.find(path);
    if (it != texture_locations.end()) //Exists
        return it->second;

    //: Load the image
    auto mode = [ch](){
        if (ch == TEXTURE_CHANNELS_G)
            return STBI_grey;
        if (ch == TEXTURE_CHANNELS_GA)
            return STBI_grey_alpha;
        if (ch == TEXTURE_CHANNELS_RGB)
            return STBI_rgb;
        return STBI_rgb_alpha;
    }();
    
    Vec2<> size;
    int real_ch;
    ui8* pixels = stbi_load(path.c_str(), &size.x, &size.y, &real_ch, mode);
    
    //: Create texture
    TextureID tex_id = API::registerTexture(api, size, ch, pixels);
    texture_locations[path] = tex_id;
    
    //: Free from memory
    stbi_image_free(pixels);
    
    return tex_id;
}

void Graphics::bindTexture(DrawID draw_id, TextureID tex_id) {
    //---Bind texture---
    //      Update descriptor sets to reflect the new texture attached to this draw id
    API::draw_data.at(draw_id).texture_id = tex_id;
    API::updateDescriptorSets(api, &API::draw_data.at(draw_id));
}

void Graphics::unbindTexture(DrawID draw_id) {
    //---Unbind texture---
    API::draw_data.at(draw_id).texture_id = std::nullopt;
    API::updateDescriptorSets(api, &API::draw_data.at(draw_id));
}

void Graphics::draw(const DrawID draw_id, glm::mat4 model) {
    //---Draw---
    //      Checks if the draw id provided and all the data attached are valid, and adds it to the correct draw queue
    DrawQueueData queue_data = {&API::draw_data.at(draw_id), model};
    if (queue_data.first == nullptr)
        log::error("Tried to draw an object with a null pointer draw data");
    
    const DrawBufferData* buffer = &API::draw_buffer_data.at(queue_data.first->buffer_id);
    if (buffer == nullptr)
        log::error("Tried to draw an object with a null pointer draw buffer");
    
    const TextureData* tex;
    if (queue_data.first->texture_id.has_value()) {
        tex = &API::texture_data.at(queue_data.first->texture_id.value());
        if (tex == nullptr)
            log::error("Tried to draw an object with an invalid texture id");
    } else {
        tex = &no_texture;
    }
    
    API::draw_queue[queue_data.first->shader][buffer][tex].push_back(queue_data);
}

void Graphics::updateCameraView(CameraData &cam) {
    //: Example of view matrix
    cam.view = glm::translate(glm::mat4(1.0f), glm::vec3(-camera.pos.x, -camera.pos.y, -camera.pos.z));
}

void Graphics::updateCameraProjection(CameraData &cam) {
    Vec2<float> resolution = (cam.proj_type & PROJECTION_SCALED) ? Config::resolution.to<float>() : win.size.to<float>();
    
    //: Orthographic (2D)
    if (cam.proj_type & PROJECTION_ORTHOGRAPHIC)
        cam.proj = glm::ortho(0.0f, resolution.x, 0.0f, resolution.y, -10000.0f, 10000.0f);
    
    //: Perspective (3D)
    if (cam.proj_type & PROJECTION_PERSPECTIVE)
        cam.proj = glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 10000.0f);
    
    cam.proj[1][1] *= -viewport_y;
    
    //: None (Vertex passthrough)
    if (cam.proj_type & PROJECTION_NONE)
        cam.proj = glm::mat4(1.0f);
}
