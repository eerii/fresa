//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_vertex.h"
#include "log.h"

using namespace Verse;
using namespace Graphics;

std::vector<VertexAttributeDescription> Vertex::getAttributeDescriptions() {
    std::vector<VertexAttributeDescription> attribute_descriptions = {};
    
    log::graphics("Creating attribute descriptions...");
    
    ui32 offset = 0;
    Reflection::forEach(VertexData{}, [&](auto &&x, ui8 level, const char* name){
        if (level == 1) { //Is a member of VertexData
            int i = (int)attribute_descriptions.size();
            attribute_descriptions.resize(i + 1);
            
            attribute_descriptions[i].binding = 0;
            attribute_descriptions[i].location = i;
            
            int size = sizeof(x);
            if (size % 4 != 0 or size < 4 or size > 16)
                log::error("Vertex data has an invalid size %d", size);
            attribute_descriptions[i].format = (VertexFormat)(size / 4);
            
            attribute_descriptions[i].offset = offset;
            
            log::graphics(" - Attribute %s [%d] - Format : %d - Size : %d - Offset %d", name, i, attribute_descriptions[i].format, size, offset);
            
            offset += sizeof(x);
        }
    });

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
