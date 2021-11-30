//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "log.h"
#include "reflection.h"

#include <glm/glm.hpp>
#include <vector>

namespace Verse::Graphics
{
    //Needs to be ordered the same way as the shader
    struct VertexData {
        Serialize(VertexData, pos, color, uv);
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };

    enum VertexFormat {
        VERTEX_FORMAT_R_F = 1,
        VERTEX_FORMAT_RG_F = 2,
        VERTEX_FORMAT_RGB_F = 3,
        VERTEX_FORMAT_RGBA_F = 4,
    };

    struct VertexAttributeDescription {
        ui32 binding;
        ui32 location;
        VertexFormat format;
        ui32 offset;
    };

    #ifdef USE_OPENGL
    struct VertexArrayData {
        ui32 id_;
        std::vector<VertexAttributeDescription> attributes;
    };
    #endif

    namespace API
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
    }
}
