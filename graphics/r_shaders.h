//* shaders
//      includes the spv shader loading and compilation functions, as well as the datatypes for representing them
#pragma once

#include "r_api.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: interfaces for vulkan objects (allows for making this file mostly api agnostic in the future)
    using IShaderStageFlagBits = VkShaderStageFlagBits;
    using IDescriptorType = VkDescriptorType;
    using IShaderModule = VkShaderModule;
    using IDescriptorSetLayout = VkDescriptorSetLayout;
    using IDescriptorSet = VkDescriptorSet;
    using IDescriptorPool = VkDescriptorPool;
    using IPipelineLayout = VkPipelineLayout;
    using IPipeline = VkPipeline;

    //: shader stages
    enum struct ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };
    struct ShaderStageData {
        str_view extension;
        IShaderStageFlagBits stage;
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
        IDescriptorType descriptor_type;
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
        IDescriptorType descriptor_type;
        ShaderStage stage_flags;
        str name;
    };

    //: shader module
    //      encapsulates a vulkan shader module (the compiled shader), as well as the stage it belongs to and reflection data
    struct ShaderModule {
        IShaderModule module = VK_NULL_HANDLE;
        ShaderStage stage;
        std::vector<DescriptorLayoutBinding> bindings;
    };

    //: descriptor set
    //      contains a descriptor set list (one per frame in flight) and its layout and set index
    struct DescriptorSet {
        ui32 set_index;
        IDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::array<IDescriptorSet, engine_config.vk_frames_in_flight()> descriptors;
    };

    //: shader pass
    //      it is a representation of a built vulkan pipeline. it contains the pipeline, its layout and the descriptor sets and modules needed to use it
    //      this is the final encapsulation step for a shader, and contains all the necessary information to render with it
    struct ShaderPass {
        std::vector<ShaderModule> stages;
        std::vector<DescriptorSet> descriptors;
        IPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        IPipeline pipeline = VK_NULL_HANDLE;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace shader
    {
        //  implemented in r_shaders.cpp

        //: read spirv code from .spv file
        std::vector<ui32> readSPIRV(str_view name, ShaderStage stage);

        //: create spirv cross compiler from the code
        //      this is used to get the reflection data from the shader
        //      it has to revert all the bits in the shader code since spirv cross takes it that way
        spv_c::CompilerGLSL createCompiler(std::vector<ui32> code);

        //: use spirv reflection to automatically get the descriptor set layout bindings
        std::vector<DescriptorLayoutBinding> getDescriptorBindings(const spv_c::CompilerGLSL& compiler, ShaderStage stage);

        //  implemented in r_api_*.cpp

        //: create shader module object
        ShaderModule createModule(str_view name, ShaderStage stage);

        //: create descriptor pool
        IDescriptorPool createDescriptorPool();

        //: create descriptor sets
        std::vector<DescriptorSet> allocateDescriptorSets(const std::vector<ShaderModule> &stages);

        //: create a shader pass from the given shader name
        ShaderPass createPass(str_view name);
    }
}