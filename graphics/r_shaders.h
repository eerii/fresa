//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_opengl.h"
#include "r_vulkan.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Shader stages, represnet the different parts of the rendering or compute pipelines
    enum ShaderStage {
        SHADER_STAGE_VERTEX    =  IF_VULKAN(VK_SHADER_STAGE_VERTEX_BIT)    IF_OPENGL(GL_VERTEX_SHADER),
        SHADER_STAGE_FRAGMENT  =  IF_VULKAN(VK_SHADER_STAGE_FRAGMENT_BIT)  IF_OPENGL(GL_FRAGMENT_SHADER),
        SHADER_STAGE_COMPUTE   =  IF_VULKAN(VK_SHADER_STAGE_COMPUTE_BIT)   IF_OPENGL(-1),
        SHADER_STAGE_GEOMETRY  =  IF_VULKAN(VK_SHADER_STAGE_GEOMETRY_BIT)  IF_OPENGL(GL_GEOMETRY_SHADER),
    };
    
    //: Shader descriptors, the allowed glsl descriptor types on shaders, used for automatic reflection with SPIRV-cross
    enum ShaderDescriptorType {
        DESCRIPTOR_UNIFORM           =  IF_VULKAN(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)          IF_OPENGL(GL_UNIFORM_BUFFER),
        DESCRIPTOR_STORAGE           =  IF_VULKAN(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)          IF_OPENGL(GL_SHADER_STORAGE_BUFFER),
        DESCRIPTOR_IMAGE_SAMPLER     =  IF_VULKAN(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)  IF_OPENGL(GL_TEXTURE_2D),
        DESCRIPTOR_INPUT_ATTACHMENT  =  IF_VULKAN(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)        IF_OPENGL(GL_TEXTURE_2D),
    };
    
    //: API indepentent descriptor layouts
    using IDescriptorLayoutBinding  =  IF_VULKAN(VkDescriptorSetLayoutBinding)  IF_OPENGL(GlDescriptorSetLayoutBinding);
    using IDescriptorLayout         =  IF_VULKAN(VkDescriptorSetLayout)         IF_OPENGL(std::vector<GlDescriptorSetLayoutBinding>);
    
    //: API independent shader module, holds the API data representation of a shader stage
    using IShaderModule  =  IF_VULKAN(VkShaderModule)  IF_OPENGL(ui8);
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
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
    //      Holds the list of modules, their reflected and combined descriptor layouts and the pipeline it is attached to
    struct ShaderPass {
        std::vector<ShaderModule> stages;
        std::map<ui32, IDescriptorLayout> descriptor_layouts;
        
        //LEGACY - TODO: REPLACE
        bool is_draw;
        bool is_instanced;
    };
    
    //: Shader lists (Temporary)
    //      Hold the multiple shader passes with their associated id (for now it is their name)
    //      TODO: Refactor this, gather them in different groups, like draw_opaque, draw_transparent, post, compute...
    inline std::map<ShaderID, ShaderPass> shaders;
    inline std::map<ShaderID, ShaderPass> compute_shaders;
    
    //: SPIRV-cross compiler and resources alias
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;
    
    namespace Shader {
        //---------------------------------------------------
        //: Systems
        //---------------------------------------------------
        
        //: Load a .spv file into code
        std::vector<ui32> readSPIRV(str name, str extension);
        
        //: Create a shader module from SPIRV code
        ShaderModule createModule(str name, ShaderStage stage);
        
        //: Create a shader pass from one or more shader modules, also does reflection on descriptor layouts
        ShaderPass createPass(str name);
        
        //: Load all the shaders from the res/shaders folder into the corresponding shader list
        void loadShaders();
        
        //: Get a module from a shader pass by specifying its stage
        const ShaderModule* getPassModule(const ShaderPass &pass, ShaderStage stage);
        
        //: Creates a SPIRV-cross compiler using the loaded code
        ShaderCompiler getCompiler(std::vector<ui32> code);
        
        //: Uses SPIRV reflection to automatically create descriptor layout bindings
        std::map<ui32, std::vector<IDescriptorLayoutBinding>> getDescriptorLayoutBindings(const ShaderModule &module);
        
        //: Obtains the compute x,y,z group size using reflection
        std::array<ui32, 3> getComputeGroupSize(const ShaderPass &pass);
        
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
        
        inline const std::map<ShaderDescriptorType, str> descriptor_names {
            {DESCRIPTOR_UNIFORM, "Uniform"},
            {DESCRIPTOR_STORAGE, "Storage"},
            {DESCRIPTOR_IMAGE_SAMPLER, "Image Sampler"},
            {DESCRIPTOR_INPUT_ATTACHMENT, "Input Attachment"},
        };
    }
    
    //---------------------------------------------------
    //: API dependen systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        IShaderModule createInternalShaderModule(const std::vector<ui32> &code, ShaderStage stage);
    }
}
