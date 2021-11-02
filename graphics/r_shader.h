//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//thank you so much to
//- augusto ruiz (https://github.com/AugustoRuiz/sdl2glsl)
//- the cherno (https://youtu.be/71BLZwRGUJE)
//for the help with this part c:

#pragma once

#include "dtypes.h"

#ifdef USE_OPENGL
#include "r_opengl.h"
#endif

#ifdef USE_VULKAN
#include "r_vulkan.h"
#endif

#include "r_shaderdata.h"

#include <optional>

namespace Verse::Graphics::Shader
{
    #if defined USE_OPENGL
    
    ui8 compileShaderGL(const char* source, ui32 shader_type);
    ui8 compileProgramGL(str vertex_file, str fragment_file);

    ShaderData create(str vertex, str frag, std::vector<str> loc);
    void validate(ShaderData &shader);
    
    #elif defined USE_VULKAN

    std::vector<char> readSPIRV(std::string filename);
    ShaderCode readSPIRV(ShaderData &data);

    VkShaderModule createShaderModule(std::vector<char> &code, VkDevice &device);
    ShaderStages createShaderStages(ShaderCode &code, VkDevice &device);

    std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfo(ShaderStages &stages);
    void destroyShaderStages(VkDevice &device, ShaderStages &stages);

    ShaderCompiler getShaderCompiler(std::vector<char> &code);
    #endif
}
