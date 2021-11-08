//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_api.h"
#include "r_drawdata.h"

namespace Verse::Graphics::Renderer
{
    RenderData create(WindowData &win, Vec2<> resolution, bool vsync = true);

    void draw(const TextureData &tex, DrawID draw_id);

    void update();

    void test(WindowData &win, RenderData &render);

    void clean(RenderData &render);
}
