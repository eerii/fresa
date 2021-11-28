//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "log.h"
#include "r_vertexdata.h"

#ifdef USE_VULKAN
#include "r_vulkan.h"
#include <array>
#endif

namespace Verse::Graphics::Vertex
{
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VertexAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attribute_descriptions = {};
        
        log::graphics("");
        log::graphics("Creating attribute descriptions...");
        
        ui32 offset = 0;
        Reflection::forEach(V{}, [&](auto &&x, ui8 level, const char* name){
            if (level == 1) {
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
        
        log::graphics("");
        return attribute_descriptions;
    }

    #ifdef USE_VULKAN
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VkVertexInputBindingDescription> getBindingDescriptionVK() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        
        VkVertexInputBindingDescription v;
        v.binding = 0;
        v.stride = sizeof(V);
        v.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        binding_descriptions.push_back(v);
        
        return binding_descriptions;
    }

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsVK() {
        std::vector<VertexAttributeDescription> attr_v = getAttributeDescriptions<V>();
        std::vector<VkVertexInputAttributeDescription> attr;
        attr.resize(attr_v.size());
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

    //Expand to allow multiple vertex descriptions, use it so the user can pass multiple
    //template arguments and they will be created for each of them
}
