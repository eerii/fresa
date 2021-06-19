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
    
    tex->tex_id = (int)Graphics::Renderer::createTexture(tex->tex, w, h);
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
        tex->tex_id.push_back((int)Graphics::Renderer::createTexture(t, w, h));
    }
}

void Graphics::Texture::loadTexture(str path, ui32 &tex_id) {
    if (not checkPath(path)) {
        log::error("The texture path does not exist!");
        return;
    }
    
    int w, h, ch;
    ui8* tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex_id = (ui32)Graphics::Renderer::createTexture(tex, w, h);
}

void Graphics::Texture::createPerlinNoise(Vec2 size, Vec2 offset, float freq, int levels, ui8* noise_data, ui32 &tex_id) {
    Math::perlinNoise(size, offset, freq, 4, noise_data);
    
#ifndef __EMSCRIPTEN__
    tex_id = (ui32)Graphics::Renderer::createTexture(noise_data, size.x, size.y, false);
#else
    tex_id = (ui32)Graphics::Renderer::createTexture(noise_data, size.x, size.y, true);
#endif
}

void Graphics::Texture::createGradient(int size, ui32 &tex_id) {
    ui8 gradient[size];
    float step = 255.0f / (float)(size-1);
    
    for (int i = 0; i < size; i++) {
        gradient[i] = (ui8)(step * i);
    }
    
    tex_id = (ui32)Graphics::Renderer::createTexture(gradient, size, 1, false);
}
