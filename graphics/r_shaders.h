//* shaders
//      includes the spv shader loading and compilation functions, as well as the datatypes for representing them
#pragma once

#include "r_types.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: shader types
    enum struct ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    //: shader extensions
    constexpr auto shader_extensions = std::to_array<str_view>({ ".vert", ".frag", ".comp" });

    // ·············
    // · DATATYPES ·
    // ·············

    //: ...

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace shader
    {
        //: read spirv code
        std::vector<ui32> readSPIRV(str name, ShaderStage type);
    }
}