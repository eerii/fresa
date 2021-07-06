//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//thank you so much to
//- augusto ruiz (https://github.com/AugustoRuiz/sdl2glsl)
//- the cherno (https://youtu.be/71BLZwRGUJE)
//for the help with this part c:

#pragma once

#include "dtypes.h"
#include "r_shaderdata.h"

#ifdef USE_VULKAN
#include "r_vulkan.h"
#endif

#include <optional>

namespace Verse::Graphics::Shader
{
#ifdef USE_OPENGL
    ui8 compileShaderGL(const char* source, ui32 shader_type);
    ui8 compileProgramGL(str vertex_file, str fragment_file);
    void validateProgramGL(ui8 pid);
#endif

#ifdef USE_VULKAN
    struct ShaderStages {
        std::optional<VkShaderModule> vert;
        std::optional<VkShaderModule> frag;
        std::optional<VkShaderModule> compute;
        std::optional<VkShaderModule> geometry;
    };

    std::vector<char> readSPIRV(std::string filename);
    VkShaderModule createShaderModule(std::vector<char> &code, VkDevice &device);
    std::vector<VkPipelineShaderStageCreateInfo> createShaderStageInfo(ShaderStages &stages);
#endif
}
