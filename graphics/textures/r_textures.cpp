//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "r_textures.h"

#include <filesystem>

#include "fmath.h"
#include "log.h"

#include "r_renderer.h"
#include "r_opengl.h"
#include "r_vulkan.h"

using namespace Verse;
using namespace Graphics;

bool checkPath(str path) {
    std::filesystem::path f{path};
    return std::filesystem::exists(f);
}

TextureData Texture::load(str path, Channels ch) {
    TextureData tex;
    
    //Load the image
    //---
    if (not checkPath(path))
        log::error("The texture path does not exist!");
    
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
    
    ui8* pixels = stbi_load(path.c_str(), &tex.w, &tex.h, &tex.ch, mode);
    //---
    
    //Create the texture
    //---
    
    //---
    
    stbi_image_free(pixels);
    
    return tex;
}





//TODO: DEPRECATE
//----------------------------------------
void Graphics::Texture::loadTexture(str path, Component::Texture* tex) {
    if (not checkPath(path)) {
        log::error("The texture path does not exist!");
        return;
    }
    
    int w, h, ch;
    ui8* pixels = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    createTexture(pixels, tex->data, w, h);
    
    tex->data.w = w;
    tex->data.h = h;
}

void Graphics::Texture::loadTexture(std::vector<str> path, Component::Tilemap* tile) {
    tile->tex_data = std::vector<TextureData>(path.size());
    
    int w, h, ch;
    int i = 0;
    for (str p : path) {
        if (not checkPath(p)) {
            log::error("The texture path does not exist!");
            return;
        }
        
        ui8* pixels = stbi_load(p.c_str(), &w, &h, &ch, STBI_rgb_alpha);
        
        createTexture(pixels, tile->tex_data[i], w, h);
        i++;
    }
}

void Graphics::Texture::createTexture(ui8* tex, TextureData &data, bool rgba) {
    createTexture(tex, data, data.w, data.h, rgba);
}

#if defined USE_OPENGL

void Graphics::Texture::createTexture(ui8* tex, TextureData &data, int w, int h, bool rgba) {
    ui32 tex_id;
    
    glGenTextures(1, &tex_id);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    if (rgba) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    }
    else {
    #ifndef __EMSCRIPTEN__
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, tex);
    #else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex);
    #endif
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  
    data.id_ = tex_id;
}

#elif defined USE_VULKAN

void Graphics::Texture::createTexture(ui8* tex, TextureData &data, int w, int h, bool rgba) {
    log::warn("Texture creation not implemented yet (r_textures)");
}

#endif

void Graphics::Texture::createPerlinNoise(ui8* noise_data, TextureData &tex_data, Vec2<> size, Vec2<> offset, float freq, int levels) {
    Math::perlinNoise(size, offset, freq, levels, noise_data);
    createTexture(noise_data, tex_data, size.x, size.y, false);
}
//----------------------------------------
