//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <array>

#include "config.h"
#include "r_textures.h"

#define BORDER_WIDTH 2

namespace Verse::Graphics::Renderer
{
    void create(Config &c, SDL_Window* window);
    
    void renderTexture(Config &c, TextureData &data);
    void renderTilemap(Config &c, TextureData &data);
    void renderNoise(Config &c, ui32 &noise_tex, ui32 &mask_tex, glm::mat4 model, float* vertices, float* noise_vertices, int layer);
    void renderText(Config &c, ui32 &tex_id, glm::mat4 model, float* vertices, int layer,
                    float r = 1.0f, float g = 1.0f, float b = 1.0f, bool same_color = true);
    void render3D(Config &c, float* vertices, int size);

    void render(Config &c);
    void renderPost(Config &c);
    void renderCam(Config &c);
    void renderWindow(Config &c);

    void renderTest(Config &c);
    void renderDebugCollider(Config &c, Rect2 col, bool colliding);

    void present(SDL_Window* window);
    void clear(Config &c);
    
    void destroy();

    void createFramebuffer(Config &c, ui32 &fb, ui32 &tex, Vec2 res);
    void createDepthFramebuffer(Config &c, ui32 &fb, ui32 &tex, ui32 &d_tex, Vec2 res);

    glm::mat4 matModel2D(Vec2 pos, Vec2 size, float rotation = 0.0f);
    glm::mat4 matModel2D(Rect2 rect, float rotation = 0.0f);
}
