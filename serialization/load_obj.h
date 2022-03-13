//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_dtypes.h"

namespace Fresa::Serialization
{
    struct VerticesOBJ {
        std::vector<Graphics::VertexOBJ> vertices;
        std::vector<ui16> indices;
    };
    
    VerticesOBJ loadOBJ(str file);
}
