//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_shader.h"

#include <fstream>
#include <vector>

#include "log.h"

using namespace Verse;
using namespace Graphics;

VkShaderModule Shader::createShaderModule(VkDevice device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const ui32*>(code.data());
    
    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
        log::error("Error creating a Vulkan Shader Module");
        
    return shader_module;
}

ShaderStages Shader::createShaderStages(VkDevice device, const ShaderCode &code) {
    ShaderStages stages;
    
    if (code.vert.has_value())
        stages.vert = createShaderModule(device, code.vert.value());
    if (code.frag.has_value())
        stages.frag = createShaderModule(device, code.frag.value());
    if (code.compute.has_value())
        stages.compute = createShaderModule(device, code.compute.value());
    if (code.geometry.has_value())
        stages.geometry = createShaderModule(device, code.geometry.value());
    
    return stages;
}

std::vector<VkPipelineShaderStageCreateInfo> Shader::getShaderStageInfo(const ShaderStages &stages) {
    std::vector<VkPipelineShaderStageCreateInfo> info;
    
    if (stages.vert.has_value()) {
        VkPipelineShaderStageCreateInfo vert_stage_info = {};
        
        vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_stage_info.module = stages.vert.value();
        vert_stage_info.pName = "main";
        vert_stage_info.pSpecializationInfo = nullptr; //You can customize shader variable values at compile time
        
        info.push_back(vert_stage_info);
    }
    
    if (stages.frag.has_value()) {
        VkPipelineShaderStageCreateInfo frag_stage_info = {};
        
        frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_stage_info.module = stages.frag.value();
        frag_stage_info.pName = "main";
        
        info.push_back(frag_stage_info);
    }
    
    if (stages.compute.has_value()) {
        VkPipelineShaderStageCreateInfo compute_stage_info = {};
        
        compute_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        compute_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        compute_stage_info.module = stages.compute.value();
        compute_stage_info.pName = "main";
        
        info.push_back(compute_stage_info);
    }
    
    if (stages.geometry.has_value()) {
        VkPipelineShaderStageCreateInfo geometry_stage_info = {};
        
        geometry_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geometry_stage_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geometry_stage_info.module = stages.geometry.value();
        geometry_stage_info.pName = "main";
        
        info.push_back(geometry_stage_info);
    }
    
    return info;
}

void Shader::destroyShaderStages(VkDevice device, const ShaderStages &stages) {
    if (stages.vert.has_value())
        vkDestroyShaderModule(device, stages.vert.value(), nullptr);
    if (stages.frag.has_value())
        vkDestroyShaderModule(device, stages.frag.value(), nullptr);
    if (stages.compute.has_value())
        vkDestroyShaderModule(device, stages.compute.value(), nullptr);
    if (stages.geometry.has_value())
        vkDestroyShaderModule(device, stages.geometry.value(), nullptr);
}

#endif
