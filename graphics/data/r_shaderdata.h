//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>

#include "spirv_glsl.hpp" //SPIRV-cross for reflection

namespace Verse::Graphics
{
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;

    struct ShaderLocations {
        std::optional<str> vert;
        std::optional<str> frag;
        std::optional<str> compute;
        std::optional<str> geometry;
    };

    struct ShaderCode {
        std::optional<std::vector<char>> vert;
        std::optional<std::vector<char>> frag;
        std::optional<std::vector<char>> compute;
        std::optional<std::vector<char>> geometry;
    };

    #ifdef USE_VULKAN
    struct ShaderStages {
        std::optional<VkShaderModule> vert;
        std::optional<VkShaderModule> frag;
        std::optional<VkShaderModule> compute;
        std::optional<VkShaderModule> geometry;
    };
    #endif

    struct ShaderData {
        ShaderLocations locations;
        ShaderCode code;
    #if defined USE_OPENGL
        ui8 pid;
        std::map<str, ui32> uniforms;
    #elif defined USE_VULKAN
        ShaderStages stages;
    #endif
    };
}
