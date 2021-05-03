//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <glm/glm.hpp>
#include <array>

#include "dtypes.h"
#include "log.h"
#include "config.h"
#include "scene.h"

namespace Verse::Graphics::Renderer
{
    void create(Config &c, SDL_Window* window);

    ui32 createTexture(ui8* tex, int w, int h);
    void renderTexture(ui32 &tex_id, Rect &src, Rect &dst, ui16 frames, Config &c, bool flip);
    void prepareTilemap(Rect &dst, Config &c, std::array<float, 24> &vertices);
    void renderTilemap(ui32 &tex_id, float* vertices, int size);

    void render3D(float* vertices, int size, Config &c);

    void clear(Scene &scene, Config &c);
    void useCamera(glm::mat4 *mat, Vec2 *pos);
    void render(Config &c);
    void present(SDL_Window* window);
    void destroy();
}
