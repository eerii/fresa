//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "config.h"
#include "ecs.h"
#include "gui.h"
#include "file.h"
#include "f_time.h"

using namespace Fresa;
using namespace Graphics;

namespace {
    std::map<str, TextureID> texture_locations{};
}

bool Graphics::init() {
    //---Initialization---
    
    //: Pre configuration
    configureAPI();
    
    //: Create window
    str version = std::to_string(Config::version[0]) + "." + std::to_string(Config::version[1]) + "." + std::to_string(Config::version[2]);
    str name = Config::name + " - version " + version;
    window = Window::create(Config::window_size, name);
    
    //: Create camera
    camera = Camera::create();
    
    //: Create renderer api
    createAPI();
    
    //: Initialize GUI
    IF_GUI(Gui::init());
    
    //: Link reserved buffers
    Buffer::linkReservedBuffer("InstanceBuffer", &draw_instances.buffer);
    
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
    
    //: Build draw queue
    Draw::buildDrawQueue();
    
    //: Render
    render();
    
    //: Present
    present();
    
    return true;
}

bool Graphics::stop() {
    //---Cleanup---
    
    clean();
    return true;
}

TextureID Graphics::getTextureID(str path, Channels ch) {
    //---Create TextureID---
    
    //: Check if path exists
    File::path(path);
    
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
    
    int x, y, real_ch;
    ui8* pixels = stbi_load(path.c_str(), &x, &y, &real_ch, mode);
    
    //: Create texture
    TextureID tex_id = registerTexture(Vec2<ui16>((ui16)x, (ui16)y), ch, pixels);
    texture_locations[path] = tex_id;
    
    //: Free from memory
    stbi_image_free(pixels);
    
    return tex_id;
}
