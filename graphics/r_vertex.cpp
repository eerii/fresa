//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_vertex.h"

using namespace Verse;
using namespace Graphics;

std::vector<VertexAttributeDescription> Vertex::getAttributeDescriptions() {
    std::vector<VertexAttributeDescription> attribute_descriptions = {};
    
    attribute_descriptions.resize(2);
    
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VERTEX_FORMAT_RG_F;
    attribute_descriptions[0].offset = 0;
    
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VERTEX_FORMAT_RGB_F;
    attribute_descriptions[1].offset = sizeof(glm::vec2);

    return attribute_descriptions;
}

#ifdef USE_VULKAN

VkVertexInputBindingDescription Vertex::getBindingDescriptionVK() {
    VkVertexInputBindingDescription binding_description = {};

    binding_description.binding = 0;
    binding_description.stride = sizeof(VertexData);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptionsVK() {
    std::vector<VertexAttributeDescription> attr_v = getAttributeDescriptions();
    std::array<VkVertexInputAttributeDescription, 2> attr;
    std::transform(attr_v.begin(), attr_v.end(), attr.begin(), [](const auto &a){
        VkVertexInputAttributeDescription v;
        v.binding = a.binding;
        v.location = a.location;
        v.offset = a.offset;
        switch (a.format) {
            case VERTEX_FORMAT_R_F:
                v.format = VK_FORMAT_R32_SFLOAT; break;
            case VERTEX_FORMAT_RG_F:
                v.format = VK_FORMAT_R32G32_SFLOAT; break;
            case VERTEX_FORMAT_RGB_F:
                v.format = VK_FORMAT_R32G32B32_SFLOAT; break;
            case VERTEX_FORMAT_RGBA_F:
                v.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
            default:
                v.format = VK_FORMAT_R32_SFLOAT; break;
        }
        return v;
    });
    return attr;
}

#endif
