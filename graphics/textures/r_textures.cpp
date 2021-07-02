//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_textures.h"

#include <filesystem>

#include "fmath.h"
#include "log.h"

#include "r_renderer.h"
#include "r_opengl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Verse;

bool checkPath(str path) {
    std::filesystem::path f{path};
    return std::filesystem::exists(f);
}

void Graphics::Texture::loadTexture(str path, Component::Texture* tex) {
    if (not checkPath(path)) {
        log::error("The texture path does not exist!");
        return;
    }
    
    int w, h, ch;
    tex->tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex->tex_id = (int)createTexture(tex->tex, w, h);
}

void Graphics::Texture::loadTexture(std::vector<str> path, Component::Tilemap* tex) {
    tex->tex_id = {};
    
    int w, h, ch;
    for (str p : path) {
        if (not checkPath(p)) {
            log::error("The texture path does not exist!");
            return;
        }
        
        ui8* t = stbi_load(p.c_str(), &w, &h, &ch, STBI_rgb_alpha);
        tex->tex_id.push_back((int)createTexture(t, w, h));
    }
}

void Graphics::Texture::loadTexture(str path, ui32 &tex_id) {
    if (not checkPath(path)) {
        log::error("The texture path does not exist!");
        return;
    }
    
    int w, h, ch;
    ui8* tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex_id = (ui32)createTexture(tex, w, h);
}


ui32 Graphics::Texture::createTexture(ui8* tex, int w, int h, bool rgba) {
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
                  
    return tex_id;
}


void Graphics::Texture::createPerlinNoise(Vec2 size, Vec2 offset, float freq, int levels, ui8* noise_data, ui32 &tex_id) {
    Math::perlinNoise(size, offset, freq, levels, noise_data);
    
    glDeleteTextures(1, &tex_id);
    tex_id = createTexture(noise_data, size.x, size.y, false);
}

void Graphics::Texture::createGradient(int size, ui32 &tex_id) {
    float step = 255.0f / (float)(size-1);
    ui8 gradient[size];
    for (int i = 0; i < size; i++) {
        gradient[i] = (ui8)(step * i);
    }
    
    glDeleteTextures(1, &tex_id);
    tex_id = createTexture(gradient, size, 1, false);
}
