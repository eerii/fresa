//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "time.h"
#include "noise.h"
#include "r_textures.h"
#include "r_renderer.h"
#include "r_opengl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Verse;

#ifdef TEXTURE
void Graphics::Texture::loadTexture(str path, Component::Texture* tex) {
    int w, h, ch;
    tex->tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex->tex_id = (int)Graphics::Renderer::createTexture(tex->tex, w, h);
}
#endif

#ifdef TILEMAP
void Graphics::Texture::loadTexture(str path, Component::Tilemap* tex) {
    int w, h, ch;
    tex->tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex->tex_id = (int)Graphics::Renderer::createTexture(tex->tex, w, h);
}
#endif

void Graphics::Texture::loadTexture(str path, ui32 &tex_id) {
    int w, h, ch;
    ui8* tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex_id = (ui32)Graphics::Renderer::createTexture(tex, w, h);
}

void Graphics::Texture::createWhiteNoise(int size, ui8* noise_data, ui32 &tex_id) {
    srand((ui32)Time::current);
    
    int s = size + 1;
    Math::whiteNoise(s, 2, noise_data);
    
    tex_id = (ui32)Graphics::Renderer::createTexture(noise_data, s, s, false);
}

void Graphics::Texture::offsetWhiteNoise(int size, ui8* noise_data, ui32 &tex_id) {
    int s = size + 1;
    
    ui8 r = rand() % 6;
    int off = 0;
    if (r < 2)
        off = 1;
    if (r > 4)
        off = -1;
    
    std::rotate(noise_data, &noise_data[s + off], &noise_data[(s*s)-1]);
    Math::whiteNoise(s, 1, &noise_data[s*size]);
    
    tex_id = (ui32)Graphics::Renderer::createTexture(noise_data, s, s, false);
}

void Graphics::Texture::createPerlinNoise(int size, Vec2 offset, float freq, int octaves, ui32 seed, ui8* noise_data, ui32 &tex_id) {
    Math::perlinNoise(Vec2(size, size), offset, freq, octaves, seed, noise_data);
    
    tex_id = (ui32)Graphics::Renderer::createTexture(noise_data, size, size, false);
}

void Graphics::Texture::createGradient(int size, ui32 &tex_id) {
    ui8 gradient[size];
    float step = 255.0f / (float)(size-1);
    
    for (int i = 0; i < size; i++) {
        gradient[i] = (ui8)(step * i);
    }
    
    tex_id = (ui32)Graphics::Renderer::createTexture(gradient, size, 1, false);
}
