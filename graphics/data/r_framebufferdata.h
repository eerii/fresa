//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <glm/glm.hpp>

namespace Verse::Graphics
{
    struct FramebufferData {
        str name;
        ui8 render_step;
        
        glm::vec3 clear_color;
        
    #if defined USE_OPENGL
        ui32 gl_fid;
    #elif defined USE_VULKAN
            
    #endif
    };
}
