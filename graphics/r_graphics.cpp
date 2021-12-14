//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_graphics.h"
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace Fresa;
using namespace Graphics;

namespace {
    WindowData win;
    std::map<str, TextureID> texture_locations{};
}

bool Graphics::init() {
    //---Initialization---
    
    //: Pre configuration
    API::configure();
    
    //: Create window
    str version = std::to_string(Config::version[0]) + "." + std::to_string(Config::version[1]) + "." + std::to_string(Config::version[2]);
    str name = Config::name + " - Version " + version;
    win = Window::create(Config::window_size, name);
    
    //: Create renderer api
    api = API::create(win);
    
    return true;
}

bool Graphics::update() {
    //---Update---
    
    //TODO: Add drawing systems here
    
    //: Render
    API::render(api, win);
    
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
    Vec2<float> ratios = win.size.to<float>() / win.resolution.to<float>();
    win.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //: GUI adjustment
    #ifndef DISABLE_GUI
    ImGuiIO& io = ImGui::GetIO(); //TODO: Save this somewhere where we can access it later
    io.DisplaySize.x = static_cast<float>(size.x);
    io.DisplaySize.y = static_cast<float>(size.y);
    #endif
    
    //: Pass the resize command to the API
    API::resize(api, win);
}

DrawID Graphics::getDrawID_Rect(Shaders shader) {
    //---Create DrawID for a Rect---
    //      This uses the rect vertex definition and registers a new DrawID for a rect-like object
    //      Has support for multiple shaders, so it can return the correct vertex type (color, tex...) and initialize the descriptor sets
    if (shader == SHADER_DRAW_TEX) {
        static DrawBufferID rect_buffer_tex = API::registerDrawBuffer(api, Vertices::rect_vertices_texture, Vertices::rect_indices);
        return API::registerDrawData(api, rect_buffer_tex, shader); //: Descriptor sets for texture are updated when the texture is binded
    }
    if (shader == SHADER_DRAW_COLOR) {
        static DrawBufferID rect_buffer_color = API::registerDrawBuffer(api, Vertices::rect_vertices_color, Vertices::rect_indices);
        DrawID id = API::registerDrawData(api, rect_buffer_color, shader);
        API::updateDescriptorSets(api, &API::draw_data.at(id));
        return id;
    }
    log::error("Shader not supported for rect");
    return 0;
}

DrawID Graphics::getDrawID_Cube(Shaders shader) {
    //---Create DrawID for a Cube---
    //      This uses the cube vertex definition and registers a new DrawID for a cube-like object
    if (shader == SHADER_DRAW_COLOR) {
        static DrawBufferID cube_buffer_color = API::registerDrawBuffer(api, Vertices::cube_vertices_color, Vertices::cube_indices);
        DrawID id = API::registerDrawData(api, cube_buffer_color, shader);
        API::updateDescriptorSets(api, &API::draw_data.at(id));
        return id;
    }
    log::error("Shader not supported for cube");
    return 0;
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
