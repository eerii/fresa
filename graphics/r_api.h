//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

#include "r_windowdata.h"
#include "r_renderdata.h"

namespace Verse::Graphics::API
{
    void configure();
    GraphicsAPI create(WindowData &win);

    void resize(GraphicsAPI &api, WindowData &win);

    void renderTest(WindowData &win, RenderData &render);

    void clean(GraphicsAPI &api);
}
