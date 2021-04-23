//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_textures.h"
#include "r_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace Verse;

#ifdef TEXTURE
void Graphics::Texture::loadTexture(str path, Component::Texture* tex) {
    int w, h, ch;
    tex->tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex->tex_id = (int)Graphics::Renderer::GL::createTexture(tex->tex, w, h);
}
#endif

#ifdef TILEMAP
void Graphics::Texture::loadTexture(str path, Component::Tilemap* tex) {
    int w, h, ch;
    tex->tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex->tex_id = (int)Graphics::Renderer::GL::createTexture(tex->tex, w, h);
}
#endif

void Graphics::Texture::loadTexture(str path, ui32 &tex_id) {
    int w, h, ch;
    ui8* tex = stbi_load(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    
    tex_id = (ui32)Graphics::Renderer::GL::createTexture(tex, w, h);
}

