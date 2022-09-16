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
    struct ShaderStageData {
        str_view extension;
        VkShaderStageFlagBits stage;
    };
    enum struct ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE
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
    struct ShaderModule {
        VkShaderModule module;
        ShaderStage stage;
        std::vector<DescriptorLayoutBinding> bindings;
    };

    //: descriptor set
    struct DescriptorSet {
        ui32 set_index;
        VkDescriptorSetLayout layout;
        std::array<VkDescriptorSet, engine_config.vk_frames_in_flight()> descriptors;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace shader
    {
        //* spirv

        //: read spirv code from .spv file
        std::vector<ui32> readSPIRV(str name, ShaderStage stage);

        //: create spirv cross compiler from the code
        //      this is used to get the reflection data from the shader
        //      it has to revert all the bits in the shader code since spirv cross takes it that way
        spv_c::CompilerGLSL createCompiler(std::vector<ui32> code);

        //* shader modules

        //: create vulkan shader module from spirv code
        VkShaderModule createVkShader(const std::vector<ui32>& code);

        //: create shader module object
        ShaderModule createModule(str name, ShaderStage stage);

        //* descriptor sets

        //: use spirv reflection to automatically get the descriptor set layout bindings
        std::vector<DescriptorLayoutBinding> getDescriptorBindings(const spv_c::CompilerGLSL& compiler, ShaderStage stage);

        //: create vulkan descriptor set layout from the bindings
        std::unordered_map<ui32, VkDescriptorSetLayout> createDescriptorLayout(const std::vector<ShaderModule> &stages);

        //: create descriptor pool
        VkDescriptorPool createDescriptorPool();

        //* shader pass

        //: create a shader pass from one or more shader modules, also does reflection on descriptor layouts
        //- ShaderPass createPass(str name);
    }
}