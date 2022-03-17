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

void Graphics::draw_(DrawDescription &description) {
    //---Draw---
    //      Checks if the description provided and all the data attached are valid, and adds it to the correct draw queue
    
    if (not API::shaders.count(description.shader))
        log::error("The ShaderID %s is not valid", description.shader.c_str());
    
    if (description.texture != no_texture and not API::texture_data.count(description.texture))
        log::error("The TextureID %d is not valid", description.texture);
    
    if (not API::draw_uniform_data.count(description.uniform))
        log::error("The DrawUniformID %d is not valid", description.uniform);
    
    if (not API::geometry_buffer_data.count(description.geometry))
        log::error("The GeometryBufferID %d is not valid", description.geometry);
    
    //: Instanced buffer
    if (API::shaders.at(description.shader).is_instanced) {
        if (description.instance == no_instance or not API::instanced_buffer_data.count(description.instance))
            log::error("The InstancedBufferID %d is not valid", description.instance);
        API::draw_queue_instanced[description.shader][description.uniform][description.geometry].push_back(description.instance);
    }
    //: Geometry buffer
    else {
        if (description.instance != no_instance)
            log::error("The InstancedBufferID %d is not valid", description.instance);
        API::draw_queue[description.shader][description.geometry][description.texture].push_back(description.uniform);
    }
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
