//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_api.h"

namespace Verse::Graphics::Renderer
{
    RenderData create(WindowData &win, Vec2<> resolution, bool vsync = true);

    void test(WindowData &win, RenderData &render);
}
