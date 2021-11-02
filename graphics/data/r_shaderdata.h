//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>

#ifdef USE_VULKAN
#include "spirv_glsl.hpp" //SPIRV-cross for reflection
#endif

namespace Verse::Graphics
{
    #ifdef USE_VULKAN
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;

    struct ShaderStages {
        std::optional<VkShaderModule> vert;
        std::optional<VkShaderModule> frag;
        std::optional<VkShaderModule> compute;
        std::optional<VkShaderModule> geometry;
    };

    struct ShaderCode {
        std::optional<std::vector<char>> vert;
        std::optional<std::vector<char>> frag;
        std::optional<std::vector<char>> compute;
        std::optional<std::vector<char>> geometry;
    };
    #endif

    struct ShaderLocations {
        std::optional<str> vert;
        std::optional<str> frag;
        std::optional<str> compute;
        std::optional<str> geometry;
    };

    struct ShaderData {
        ShaderLocations locations;
        
    #if defined USE_OPENGL
        ui8 pid;
        std::map<str, ui8> locations;
    #elif defined USE_VULKAN
        ShaderCode code;
        ShaderStages stages;
    #endif
    };
}
