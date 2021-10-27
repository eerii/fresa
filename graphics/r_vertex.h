//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_vertexdata.h"

#ifdef USE_VULKAN
#include "r_vulkan.h"
#include <array>
#endif

namespace Verse::Graphics::Vertex
{
    std::vector<VertexAttributeDescription> getAttributeDescriptions();

    #ifdef USE_VULKAN
    VkVertexInputBindingDescription getBindingDescriptionVK();
    std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptionsVK();
    #endif
}
