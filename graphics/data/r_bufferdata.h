//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <glm/glm.hpp>

namespace Verse::Graphics
{
    struct BufferData {
        
        #if defined USE_OPENGL
        ui32 buffer_id;
        std::optional<ui32> color_texture_id;
        std::optional<ui32> depth_texture_id;
        #elif defined USE_VULKAN
                
        #endif
    };

    enum FramebufferType {
        FRAMEBUFFER_COLOR_ATTACHMENT,
        FRAMEBUFFER_DEPTH_ATTACHMENT,
        FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT
    };
    
}
