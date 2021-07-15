//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <glm/glm.hpp>

#ifdef USE_VULKAN
#include "r_vulkan.h"
#include <array>
#endif

namespace Verse::Graphics
{
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
        
    #ifdef USE_VULKAN
        static VkVertexInputBindingDescription getBindingDescription() {
            
            VkVertexInputBindingDescription binding_description = {};

            binding_description.binding = 0;
            binding_description.stride = sizeof(Vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            
            return binding_description;
        }
        
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            
            std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};
            
            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(Vertex, pos);
            
            attribute_descriptions[1].binding = 0;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(Vertex, color);

            return attribute_descriptions;
        }
    #endif
    };
}
