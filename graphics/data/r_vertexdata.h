//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "reflection.h"

#include <glm/glm.hpp>
#include <vector>

namespace Verse::Graphics
{
    struct VertexData {
        Serialize(VertexData, pos, color, uv);
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };
    //Needs to be ordered the same way as the shader

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
}
