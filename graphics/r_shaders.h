//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_opengl.h"
#include "r_vulkan.h"
#include "r_buffers.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Shader ID (for indexing the shader list)
    using ShaderID = FresaType<str, struct ShaderTag>;
    
    //: Shader stages, represnet the different parts of the rendering or compute pipelines
    enum ShaderStage {
        SHADER_STAGE_VERTEX    =  IF_VULKAN(VK_SHADER_STAGE_VERTEX_BIT)    IF_OPENGL(1 << 0),
        SHADER_STAGE_FRAGMENT  =  IF_VULKAN(VK_SHADER_STAGE_FRAGMENT_BIT)  IF_OPENGL(1 << 1),
        SHADER_STAGE_COMPUTE   =  IF_VULKAN(VK_SHADER_STAGE_COMPUTE_BIT)   IF_OPENGL(1 << 2),
        SHADER_STAGE_GEOMETRY  =  IF_VULKAN(VK_SHADER_STAGE_GEOMETRY_BIT)  IF_OPENGL(1 << 3),
    };
    using ShaderStageT = IF_VULKAN(VkShaderStageFlagBits) IF_OPENGL(ShaderStage);
    
    //: Shader descriptors, the allowed glsl descriptor types on shaders, used for automatic reflection with SPIRV-cross
    enum ShaderDescriptor {
        DESCRIPTOR_UNIFORM           =  IF_VULKAN(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)          IF_OPENGL(1 << 0),
        DESCRIPTOR_STORAGE           =  IF_VULKAN(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)          IF_OPENGL(1 << 1),
        DESCRIPTOR_IMAGE_SAMPLER     =  IF_VULKAN(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)  IF_OPENGL(1 << 2),
        DESCRIPTOR_INPUT_ATTACHMENT  =  IF_VULKAN(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)        IF_OPENGL(1 << 3),
    };
    using ShaderDescriptorT = IF_VULKAN(VkDescriptorType) IF_OPENGL(ShaderDescriptor);
    
    //: Vertex input rate
    enum VertexInputRate {
        INPUT_RATE_VERTEX    =  IF_VULKAN(VK_VERTEX_INPUT_RATE_VERTEX)    IF_OPENGL(1 << 0),
        INPUT_RATE_INSTANCE  =  IF_VULKAN(VK_VERTEX_INPUT_RATE_INSTANCE)  IF_OPENGL(1 << 1),
    };
    using VertexInputRateT = IF_VULKAN(VkVertexInputRate) IF_OPENGL(VertexInputRate);
    
    //: OpenGL counterparts to Vulkan structures
    IF_OPENGL(
        struct GlDescriptorSetLayoutBinding;
        struct GlDescriptorPoolSize;
        struct GlDescriptorPool;
        struct GlDescriptorSet;
    )
    
    //---------------------------------------------------
    //: API independent interfaces
    //---------------------------------------------------
    
    //: Shader module, holds the API data representation of a shader stage
    using IShaderModule  =  IF_VULKAN(VkShaderModule)  IF_OPENGL(ui8);
    
    //: Descriptor layouts
    struct IDescriptorLayoutBinding {
        ui32 binding;
        ui32 size;
        ui32 descriptor_count;
        ShaderDescriptor descriptor_type;
        ShaderStage stage_flags;
        str name;
    };
    using IDescriptorLayout  =  IF_VULKAN(VkDescriptorSetLayout)  IF_OPENGL(std::vector<GlDescriptorSetLayoutBinding>);
    
    //: Descriptor pools
    using IDescriptorPoolSize  =  IF_VULKAN(VkDescriptorPoolSize)  IF_OPENGL(GlDescriptorPoolSize);
    using IDescriptorPool      =  IF_VULKAN(VkDescriptorPool)      IF_OPENGL(GlDescriptorPool);
    
    //: Descriptor sets
    using IDescriptorSet  =  IF_VULKAN(VkDescriptorSet) IF_OPENGL(GlDescriptorSet);
    
    //: Pipeline
    using IPipeline  =  IF_VULKAN(VkPipeline)  IF_OPENGL(ui8);
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Descriptor pool
    inline std::vector<IDescriptorPool> descriptor_pools;
    
    //: Shader resource
    //      A binding for a specific descriptor resource item
    struct ShaderResource {
        std::variant<UniformBufferID, StorageBufferID> id;
        ShaderDescriptor type;
        ui32 count;
        str name;
    };
    
    //: Descriptor set
    //      Holds the descriptors and bindings for each glsl set, used for sending data to the shader
    struct DescriptorSet {
        ui32 set;
        IDescriptorLayout layout;
        IDescriptorPool* pool;
        std::array<IDescriptorSet, Config::frames_in_flight> descriptors;
        std::vector<IDescriptorLayoutBinding> bindings;
        std::vector<ShaderResource> resources;
    };
    
    //: Shader module
    //      Represents one shader stage (vertex, fragment, compute...)
    //      Includes the code that the API needs to call to access it (module), the full SPIRV code for reflection (code) as well as the stage label
    struct ShaderModule {
        ShaderStage stage;
        IShaderModule module;
        std::vector<ui32> code;
    };
    
    //: Shader pass
    //      Combination of multiple shader stages to create a pipeline
    //      Holds the list of modules, descriptor sets and the built pipeline
    //      The pipeline is added later for draw shaders, when shaders are read from the renderer description
    struct ShaderPass {
        std::vector<ShaderModule> stages;
        std::vector<DescriptorSet> descriptors;
        IF_VULKAN(VkPipelineLayout pipeline_layout;)
        IPipeline pipeline;
    };
    
    //: Shader lists
    //      Hold the multiple shader passes with their associated id (for now it is their name)
    enum ShaderType {
        SHADER_DRAW,
        SHADER_POST,
        SHADER_COMPUTE,
        SHADER_LAST,
    };
    inline struct ShaderList {
        std::array<std::map<ShaderID, ShaderPass>, SHADER_LAST> list;
        std::map<ShaderID, ShaderType> types;
    } shaders;
    
    namespace Shader {
        //---------------------------------------------------
        //: Systems
        //---------------------------------------------------
        
        //---Shaders---
        
        //: Load a .spv file into code
        std::vector<ui32> readSPIRV(str name, str extension);
        
        //: Create a shader module from SPIRV code
        ShaderModule createModule(str name, ShaderStage stage);
        
        //: Create a shader pass from one or more shader modules, also does reflection on descriptor layouts
        ShaderPass createPass(str name);
        
        //: Adds a ShaderPass to the shader list with the appropiate id and type
        void registerShader(str name, ShaderType type);
        
        //: Get a shader from the shader list
        const ShaderPass& getShader(ShaderID shader);
        
        //: Get a module from a shader pass by specifying its stage
        const ShaderModule& getPassModule(const ShaderPass &pass, ShaderStage stage);
        
        //---SPIRV reflection---
        
        //: Create a SPIRV-cross compiler using the loaded code
        spv_c::CompilerGLSL getCompiler(std::vector<ui32> code);
        
        //: Uses SPIRV reflection to automatically create descriptor layout bindings
        std::map<ui32, std::vector<IDescriptorLayoutBinding>> getDescriptorLayoutBindings(const ShaderModule &module);
        
        //: Obtains the compute x,y,z group size using reflection
        std::array<ui32, 3> getComputeGroupSize(const ShaderPass &pass);
        
        //---Descriptors---
        
        //: Create descriptor sets for a set of shader stages
        std::vector<DescriptorSet> createDescriptorSets(const std::vector<ShaderModule> &stages);
        
        //: Create descriptor resources
        std::vector<ShaderResource> createDescriptorResources(const std::vector<IDescriptorLayoutBinding> &bindings);
        
        //: Update uniform buffer
        template <typename UBO>
        void updateGlobalUniform(ShaderID shader, str name, UBO ubo) {
            UniformBufferID uniform = no_uniform_buffer;
            for (auto &d : getShader(shader).descriptors)
                for (auto &res : d.resources)
                    if (res.name == name) uniform = std::get<UniformBufferID>(res.id);
            
            if (uniform == no_uniform_buffer)
                log::error("The uniform name %s is invalid", name.c_str());
            
            for (int i = 0; i < Config::frames_in_flight; i++)
                Common::updateBuffer(uniform_buffers.at(uniform + i), (void*)&ubo, sizeof(ubo));
        }
        
        //---------------------------------------------------
        //: Extra definitions
        //---------------------------------------------------
        
        inline const std::map<ShaderStage, str> shader_extensions {
            {SHADER_STAGE_VERTEX, "vert"},
            {SHADER_STAGE_FRAGMENT, "frag"},
            {SHADER_STAGE_COMPUTE, "comp"},
            {SHADER_STAGE_GEOMETRY, "geom"},
        };
        
        inline const std::map<ShaderStage, str> shader_stage_names {
            {SHADER_STAGE_VERTEX, "Vertex"},
            {SHADER_STAGE_FRAGMENT, "Fragment"},
            {SHADER_STAGE_COMPUTE, "Compute"},
            {SHADER_STAGE_GEOMETRY, "Geometry"},
        };
        
        inline const std::map<ShaderDescriptor, str> descriptor_names {
            {DESCRIPTOR_UNIFORM, "Uniform"},
            {DESCRIPTOR_STORAGE, "Storage"},
            {DESCRIPTOR_IMAGE_SAMPLER, "Image Sampler"},
            {DESCRIPTOR_INPUT_ATTACHMENT, "Input Attachment"},
        };
        
        inline const ui32 descriptor_pool_max_sets = 256;
        inline const std::vector<IDescriptorPoolSize> descriptor_pool_sizes {
            {(ShaderDescriptorT)DESCRIPTOR_UNIFORM,          descriptor_pool_max_sets * 1},
            {(ShaderDescriptorT)DESCRIPTOR_STORAGE,          descriptor_pool_max_sets * 1},
            {(ShaderDescriptorT)DESCRIPTOR_IMAGE_SAMPLER,    descriptor_pool_max_sets * 1},
            {(ShaderDescriptorT)DESCRIPTOR_INPUT_ATTACHMENT, descriptor_pool_max_sets * 1},
        };
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        IShaderModule createInternalShaderModule(const std::vector<ui32> &code, ShaderStage stage);
        IDescriptorPool createDescriptorPool(const std::vector<IDescriptorPoolSize> &sizes);
        std::vector<IDescriptorSet> allocateDescriptorSets(IDescriptorLayout layout, IDescriptorPool* pool);
        
        IPipeline createGraphicsPipeline(ShaderID shader, std::vector<std::pair<str, VertexInputRate>> vertices);
        //IPipeline createComputePipeline(ShaderID shader);
        
        void linkDescriptorResources(ShaderID shader);
    }
}
