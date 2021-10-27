//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Verse::Graphics
{
    struct VertexData {
        glm::vec2 pos;
        glm::vec3 color;
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
}
