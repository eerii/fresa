//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <array>

#include "dtypes.h"
#include "log.h"
#include "config.h"
#include "scene.h"

namespace Verse::Graphics::Renderer
{
    void create(Config &c, SDL_Window* window);
    
    void renderTexture(ui32 &tex_id, Rect2 &src, Rect2 &dst, ui16 frames, Config &c, bool flip);
    void renderTilemap(ui32 &tex_id, float* vertices, int size, Config &c);
    void renderFire(Rect2 &dst, ui32 &p_tex, ui32 &f_tex);
    void render3D(float* vertices, int size, Config &c);

    void render(Config &c);
    void renderPost(Config &c);
    void renderCam(Config &c);
    void renderWindow(Config &c);

    void present(SDL_Window* window);
    void clear(Scene &scene, Config &c);
    
    void destroy();

    void bindCamera(glm::mat4 *mat, glm::mat4 *mat_e, Vec2f *pos);
    void createFramebuffer(ui32 &fb, ui32 &tex, Vec2 res, Config &c);
    ui32 createTexture(ui8* tex, int w, int h, bool rgba = true);
    void prepareTilemap(Rect2 &dst, Config &c, std::array<float, 24> &vertices);

    glm::mat4 matModel2D(Vec2 pos, Vec2 size, float rotation = 0.0f);
    glm::mat4 matModel2D(Rect2 rect, float rotation = 0.0f);
}
