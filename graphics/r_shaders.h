//* shaders
//      includes the spv shader loading and compilation functions, as well as the datatypes for representing them
#pragma once

#include "r_types.h"
#include "fresa_config.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: shader types
    struct ShaderStageData {
        str_view extension;
        VkShaderStageFlagBits stage;
    };

    enum struct ShaderStages {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    constexpr inline auto shader_stages = std::to_array<ShaderStageData>({
        {"vert", VK_SHADER_STAGE_VERTEX_BIT},
        {"frag", VK_SHADER_STAGE_FRAGMENT_BIT},
        {"comp", VK_SHADER_STAGE_COMPUTE_BIT}
    });

    // ·············
    // · DATATYPES ·
    // ·············

    //: shader resource
    //      a binding for a specific descriptor resource item
    /*struct ShaderResource {
        ui16 id;
        str name;
        VkDescriptorType type;
    };*/
    
    //: descriptor set
    //      holds the descriptors and bindings for each glsl set, used for sending data to the shader
    /*struct DescriptorSet {
        ui32 set;
        VkDescriptorSetLayout layout;
        VkDescriptorPool* pool;
        std::array<VkDescriptorSet, engine_config.vk_frames_in_flight()> descriptors;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<ShaderResource> resources;
    };*/

    //: shader pass
    //      combination of multiple shader stages to create a pipeline
    //      holds the list of modules, descriptor sets and the built pipeline
    /*struct ShaderPass {
        std::vector<VkShaderModule> stages;
        std::vector<DescriptorSet> descriptors;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
    };*/

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace shader
    {
        //: read spirv code
        std::vector<ui32> readSPIRV(str name, ShaderStages stage);

        //: create shader module from spirv code
        VkShaderModule createModule(str name, ShaderStages stage);

        //: create a shader pass from one or more shader modules, also does reflection on descriptor layouts
        //- ShaderPass createPass(str name);
    }
}