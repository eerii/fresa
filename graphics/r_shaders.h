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

    //: shader stages
    enum struct ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };
    struct ShaderStageData {
        str_view extension;
        VkShaderStageFlagBits stage;
    };
    constexpr inline auto shader_stages = std::to_array<ShaderStageData>({
        {"vert", VK_SHADER_STAGE_VERTEX_BIT},
        {"frag", VK_SHADER_STAGE_FRAGMENT_BIT},
        {"comp", VK_SHADER_STAGE_COMPUTE_BIT}
    });

    //: shader descriptor types
    enum struct ShaderDescriptor {
        UNIFORM,
        STORAGE,
        IMAGE_SAMPLER,
        INPUT_ATTACHMENT
    };
    struct ShaderDescriptorData {
        str_view name;
        VkDescriptorType descriptor_type;
    };
    constexpr inline auto shader_descriptors = std::to_array<ShaderDescriptorData>({
        {"uniform", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {"storage", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {"image_sampler", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {"input_attachment", VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT}
    });

    // ·············
    // · DATATYPES ·
    // ·············

    //: descriptor set layout binding
    //      contains information about a shader binding, populated by spirv reflection
    //      can be translated to a vkDescriptorSetLayoutBinding, but includes useful extra information
    struct DescriptorLayoutBinding {
        ui32 binding;
        ui32 set;
        ui32 size;
        ui32 descriptor_count;
        VkDescriptorType descriptor_type;
        ShaderStage stage_flags;
        str name;
    };

    //: shader module
    //      encapsulates a vulkan shader module (the compiled shader), as well as the stage it belongs to and reflection data
    struct ShaderModule {
        VkShaderModule module;
        ShaderStage stage;
        std::vector<DescriptorLayoutBinding> bindings;
    };

    //: descriptor set
    //      contains a descriptor set list (one per frame in flight) and its layout and set index
    struct DescriptorSet {
        ui32 set_index;
        VkDescriptorSetLayout layout;
        std::array<VkDescriptorSet, engine_config.vk_frames_in_flight()> descriptors;
    };

    //: shader pass
    //      it is a representation of a built vulkan pipeline. it contains the pipeline, its layout and the descriptor sets and modules needed to use it
    //      this is the final encapsulation step for a shader, and contains all the necessary information to render with it
    struct ShaderPass {
        std::vector<ShaderModule> stages;
        std::vector<DescriptorSet> descriptors;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace shader
    {
        //: create shader module object
        ShaderModule createModule(str_view name, ShaderStage stage);

        //: create descriptor pool
        VkDescriptorPool createDescriptorPool();

        //: create descriptor sets
        std::vector<DescriptorSet> allocateDescriptorSets(const std::vector<ShaderModule> &stages);

        //: create a shader pass from the given shader name
        ShaderPass createPass(str_view name);
    }
}