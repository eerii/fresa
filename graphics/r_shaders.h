//* shaders
//      includes the spv shader loading and compilation functions, as well as the datatypes for representing them
#pragma once

#include "r_api.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: check for required interface definitions
    static_assert(requires { typename IShaderModule; },         "graphics api interface 'IShaderModule' not defined");
    static_assert(requires { typename IDescriptorSetLayout; },  "graphics api interface 'IDescriptorSetLayout' not defined");
    static_assert(requires { typename IDescriptorSet; },        "graphics api interface 'IDescriptorSet' not defined");
    static_assert(requires { typename IDescriptorPool; },       "graphics api interface 'IDescriptorPool' not defined");
    static_assert(requires { typename IPipelineLayout; },       "graphics api interface 'IPipelineLayout' not defined");
    static_assert(requires { typename IPipeline; },             "graphics api interface 'IPipeline' not defined");

    //: shader stages
    enum struct ShaderStage {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };
    constexpr auto shader_stage_extensions = std::to_array({
        "vert",
        "frag",
        "comp"
    });

    //: shader descriptor types
    enum struct ShaderDescriptor {
        UNIFORM,
        STORAGE,
        IMAGE_SAMPLER,
        INPUT_ATTACHMENT
    };
    constexpr auto shader_descriptor_names = std::to_array({
        "uniform",
        "storage",
        "image_sampler",
        "input_attachment"
    });

    //: check required values for enum types
    static_assert(shader_stage_values.size() == shader_stage_extensions.size(),
                  "shader stage values have not been defined in the implementation");
    static_assert(shader_descriptor_values.size() == shader_descriptor_names.size(),
                  "shader descriptor values have not been defined in the implementation");

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
        ShaderDescriptor descriptor_type;
        ShaderStage stage_flags;
        str name;
    };

    //: shader module
    //      encapsulates a vulkan shader module (the compiled shader), as well as the stage it belongs to and reflection data
    struct ShaderModule {
        IShaderModule module;
        ShaderStage stage;
        std::vector<DescriptorLayoutBinding> bindings;
    };

    //: descriptor set
    //      contains a descriptor set list (one per frame in flight) and its layout and set index
    struct DescriptorSet {
        ui32 set_index;
        IDescriptorSetLayout layout;
        std::array<IDescriptorSet, engine_config.vk_frames_in_flight()> descriptors;
    };

    //: shader pass
    //      it is a representation of a built vulkan pipeline. it contains the pipeline, its layout and the descriptor sets and modules needed to use it
    //      this is the final encapsulation step for a shader, and contains all the necessary information to render with it
    struct ShaderPass {
        std::vector<ShaderModule> stages;
        std::vector<DescriptorSet> descriptors;
        IPipelineLayout pipeline_layout;
        IPipeline pipeline;
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