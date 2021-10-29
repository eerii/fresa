//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <glm/glm.hpp>

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace Verse::Graphics
{
    struct BufferData {
        #if defined USE_OPENGL
        ui32 id_;
        #elif defined USE_VULKAN
        VkBuffer buffer;
        VkDeviceMemory memory;
        #endif
    };
}
