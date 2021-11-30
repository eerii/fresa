//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_dtypes.h"
#include "r_api.h"

#include "config.h"

#define BORDER_WIDTH 2

namespace Verse::Graphics::Renderer
{
    void create(GraphicsAPI *api, Config &c);
    
    void renderTexture(Config &c, TextureData &data);
    void renderTilemap(Config &c, TextureData &data);
    void renderNoise(Config &c, TextureData &mask_data, TextureData &noise_data);
    void renderText(Config &c, TextureData &data, float r = 1.0f, float g = 1.0f, float b = 1.0f, bool same_color = true);

    void renderLight(Config &c);
    void renderPost(Config &c);
    void renderCam(Config &c);
    void renderWindow(Config &c);

    void renderTest(WindowData &win);
    void renderDebugCollider(Config &c, Rect2<> col, bool colliding);
    void renderDebugColliderCircle(Config &c, Vec2<> pos, ui16 radius, bool colliding);

    void present(SDL_Window* window);
    void clear(Config &c);
    
    void destroy();

    void onResize(WindowData &win);

    void createFramebuffer(Config &c, ui32 &fb, ui32 &tex, Vec2<> res);
    void createDepthFramebuffer(Config &c, ui32 &fb, ui32 &tex, ui32 &d_tex, Vec2<> res);

    void toggleDepthTest(bool enable);

    glm::mat4 matModel2D(Vec2<> pos, Vec2<> size, float rotation = 0.0f);
    glm::mat4 matModel2D(Rect2<> rect, float rotation = 0.0f);
}
