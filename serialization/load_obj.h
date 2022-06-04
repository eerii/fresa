//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "r_types.h"

namespace Fresa::Serialization
{
    struct VerticesOBJ {
        std::vector<Graphics::VertexOBJ> vertices;
        std::vector<ui32> indices; //: ui16 supports up to 65.536 individual vertices, for up to 4.000 million use ui32
    };
    
    VerticesOBJ loadOBJ(str file);
}
