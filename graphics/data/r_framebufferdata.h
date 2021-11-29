//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_bufferdata.h"
#include "r_drawdata.h"

namespace Verse::Graphics
{
    enum FramebufferType {
        FRAMEBUFFER_COLOR_ATTACHMENT,
        FRAMEBUFFER_DEPTH_ATTACHMENT,
        FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT
    };
    
    struct FramebufferData {
        FramebufferType type;
        std::optional<TextureData> color_texture;
        std::optional<TextureData> depth_texture;
        #ifdef USE_OPENGL
        ui32 id_;
        #endif
    };
}
