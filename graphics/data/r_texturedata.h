//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_vulkan.h"
#include <vector>
#include <glm/glm.hpp>

namespace Verse::Graphics
{
    struct TextureData {
        int w, h, ch;
        int real_ch;
    #if defined USE_OPENGL
        ui32 id_;
    #elif defined USE_VULKAN
        VkImage image;
        VkDeviceMemory memory;
        VkFormat format;
        VkImageLayout layout;
        
        VkImageView image_view;
        VkSampler sampler;
    #endif
    };
}
