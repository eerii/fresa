//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//thank you so much to
//- augusto ruiz (https://github.com/AugustoRuiz/sdl2glsl)
//- the cherno (https://youtu.be/71BLZwRGUJE)
//for the help with this part c:

#pragma once

#include "dtypes.h"

#include "r_opengl.h"
#include "r_vulkan.h"

#include "r_shaderdata.h"

#include <optional>

namespace Verse::Graphics::Shader
{
    ShaderData createShaderData(str vert = "", str frag = "", str compute = "", str geometry = "");

    std::vector<char> readSPIRV(std::string filename);
    ShaderCode readSPIRV(const ShaderLocations &locations);

    ShaderCompiler getShaderCompiler(const std::vector<char> &code);

    #if defined USE_OPENGL
    
    ui8 compileShaderGL(const char* source, ui32 shader_type);
    ui8 compileProgramGL(str vert_source = "", str frag_source = "");

    void validate(const ShaderData &shader);
    
    #elif defined USE_VULKAN

    VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
    ShaderStages createShaderStages(VkDevice device, const ShaderCode &code);

    std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfo(const ShaderStages &stages);
    void destroyShaderStages(VkDevice device, const ShaderStages &stages);
    #endif
}
