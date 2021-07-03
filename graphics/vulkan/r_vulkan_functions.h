//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

namespace Verse::Graphics::VK
{
    void acquireNextImage();

    void resetCommandBuffer();
    void beginCommandBuffer();
    void endCommandBuffer();
    void freeCommandBuffers();

    void beginRenderPass(VkClearColorValue clear_color, VkClearDepthStencilValue clear_depth_stencil);
    void endRenderPass();

    void queueSubmit();
    void queuePresent();

    void setViewport(int w, int h);
    void setScissor(int w, int h);
}

#endif
