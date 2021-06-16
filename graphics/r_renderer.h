//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <array>

#include "config.h"

#define BORDER_WIDTH 2

namespace Verse::Graphics::Renderer
{
    void create(Config &c, SDL_Window* window);
    
    void renderTexture(Config &c, ui32 &tex_id, glm::mat4 model, float* vertices, int layer);
    void renderTilemap(Config &c, ui32 &tex_id, float* vertices, int size , int layer);
    void renderFire(Config &c, Rect2 &dst, ui32 &p_tex, ui32 &g_tex, ui32 &f_tex, int layer);
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
    ui32 createTexture(ui8* tex, int w, int h, bool rgba = true);
    void prepareTilemap(Config &c, Rect2 &dst, std::array<float, 24> &vertices);

    glm::mat4 matModel2D(Vec2 pos, Vec2 size, float rotation = 0.0f);
    glm::mat4 matModel2D(Rect2 rect, float rotation = 0.0f);
}
