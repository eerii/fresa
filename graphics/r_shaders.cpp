//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_shaders.h"
#include "r_vulkan_api.h"
#include "file.h"

using namespace Fresa;
using namespace Graphics;

//---------------------------------------------------
//: Read SPIRV code
//---------------------------------------------------
std::vector<ui32> Shader::readSPIRV(str name, str extension) {
    //: Check if res/shaders/name/name.vert.spv exists
    str path = File::path("shaders/" + name + "/" + name + "." + extension + ".spv");
    
    //: Open a file stream into the shader file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        log::error("Failed to open the shader file (%s)", path.c_str());
    
    //: Create a ui32 buffer (suitable for spirv-cross)
    size_t file_size = (size_t) file.tellg();
    std::vector<ui32> buffer(file_size / sizeof(ui32));
    
    //: Read the file into the buffer
    file.seekg(0);
    file.read((char*)buffer.data(), file_size);
    
    //: Close and return
    file.close();
    return buffer;
}

//---------------------------------------------------
//: Create ShaderModule
//---------------------------------------------------
ShaderModule Shader::createModule(str name, ShaderStage stage) {
    ShaderModule module{};
    
    //: Stage
    module.stage = stage;
    
    //: Code
    module.code = readSPIRV(name, shader_extensions.at(stage));
    
    //: Module
    module.module = Common::createInternalShaderModule(module.code, module.stage);
    
    return module;
}

//---------------------------------------------------
//: Create ShaderPass
//---------------------------------------------------
ShaderPass Shader::createPass(str name) {
    ShaderPass pass{};
    
    //: Shader path
    str path = File::path("shaders/" + name + "/");
    
    //---Stages---
    
    //: Iterate through all shader stages (vert, frag, comp...)
    for (auto &f : fs::recursive_directory_iterator(path)) {
        //: Only process the .spv shaders
        if (f.path().extension() != ".spv")
            continue;
        
        //: Get the shader stage
        std::vector<str> s = split(f.path().stem().string(), ".");
        if (s.size() != 2)
            log::error("Incorrect format for %s, it must be file.extension.spv", f.path().string().c_str());
        auto it = std::find_if(shader_extensions.begin(), shader_extensions.end(), [s](const auto &ext){ return s[1] == ext.second; });
        if (it == shader_extensions.end())
            log::error("Shader extension .%s is not valid", s[1].c_str());
        ShaderStage stage = it->first;
        
        //: Add ShaderModule to ShaderPass
        pass.stages.push_back(createModule(name, stage));
    }
    
    //: Error checking
    if (pass.stages.size() == 0)
        log::error("No ShaderPass stages were found");
    
    //---Descriptor Layout---
    
    //: Get bindings for all stages
    std::map<ui32, std::vector<IDescriptorLayoutBinding>> bindings{};
    for (auto &stage : pass.stages) {
        auto stage_bindings = getDescriptorLayoutBindings(stage);
        for (auto &[set, sb] : stage_bindings)
           bindings[set].insert(bindings[set].end(), sb.begin(), sb.end());
    }
        
    //: For each descriptor set
    for (auto &[set, b] : bindings) {
        //: Merge bindings for different stages
        std::map<ui32, ui32> binding_to_index{};
        for (int i = 0; i < b.size(); i++) {
            ui32 binding = b.at(i).binding;
            
            //: Binding already exists
            if (binding_to_index.count(binding)) {
                //: Index to previous entry with the same binding
                int j = binding_to_index.at(binding);
                
                //: Error checking
                if (b.at(j).descriptorType != b.at(i).descriptorType)
                    log::error("Descriptor types for the same binding (%d) must be the same, and they are %s and %s", binding,
                               descriptor_names.at((ShaderDescriptorType)b.at(j).descriptorType).c_str(),
                               descriptor_names.at((ShaderDescriptorType)b.at(j).descriptorType).c_str());
                if (b.at(j).stageFlags == b.at(i).stageFlags)
                    log::error("It is not allowed to repeat the same binding (%d) in the same stage (%s)", binding,
                               shader_stage_names.at((ShaderStage)b.at(j).stageFlags).c_str());
                
                //: Merge stage flags
                b.at(j).stageFlags |= b.at(i).stageFlags;
            }
            //: New binding
            else {
                binding_to_index[binding] = i;
            }
        }
        
        //: Sort and create final bindings
        std::vector<IDescriptorLayoutBinding> final_bindings{};
        for (auto &[_, i] : binding_to_index)
            final_bindings.push_back(b.at(i));
        
        //: Create descriptor layout
        pass.descriptor_layouts[set] = IF_VULKAN(VK::createDescriptorSetLayout(final_bindings)) IF_OPENGL(final_bindings);
    }
    
    return pass;
}

//---------------------------------------------------
//: Load all the shaders from the res/shaders folder into ShaderPasses
//---------------------------------------------------
void Shader::loadShaders() {
    //: Shader path
    str shader_path = File::path("shaders/");
    
    //: Iterate through all the files in the shader directory
    for (auto &f : fs::recursive_directory_iterator(shader_path)) {
        //: Skip all the files or directories that are not .spv
        if (not f.path().has_extension() or f.path().extension() != ".spv")
            continue;
        
        //: Get name and real extension (.frag, .vert, ...)
        std::vector<str> file_name = split(f.path().stem().string(), ".");
        str name = file_name.at(0), extension = file_name.at(1);
        
        //: Skip if the shader is already loaded
        if (shaders.count(name) or compute_shaders.count(name))
            continue;
        
        //: Add to shader list
        if (extension == "comp")
            compute_shaders[name] = Shader::createPass(name);
        else if (extension == "vert" or extension == "frag")
            shaders[name] = Shader::createPass(name);
        else
            log::error("Shader extension not supported %s", extension.c_str());
    }
}

//---------------------------------------------------
//: Get a specific stage module from a ShaderPass
//---------------------------------------------------
const ShaderModule* Shader::getPassModule(const ShaderPass &pass, ShaderStage stage) {
    const ShaderModule* module = [&](){
        //: Find module that matches the shader stage
        for (auto &m : pass.stages) {
            if (m.stage != stage)
                continue;
            return &m;
        }
        
        //: Error checking
        log::error("You are trying to get a '%s' ShaderStage that is not present in this ShaderPass", shader_extensions.at(stage).c_str());
        return (const ShaderModule*)nullptr;
    }();
    
    return module;
}

//---------------------------------------------------
//: Get a SPIRV-cross compiler for a specific shader
//      It has to revert all the bits in the shader code since SPIRV-cross takes it that way
//      Important for shader reflection and creating descriptor sets automatically
//---------------------------------------------------
ShaderCompiler Shader::getCompiler(std::vector<ui32> code) {
    //: Revert the bytes in each ui32
    for (auto i : code) {
        ui8* istart = (ui8*)&i, *iend = (ui8*)&i + sizeof(ui32) / sizeof(ui8);
        std::reverse(istart, iend);
    }
    
    //: Create a SPIRV-cross GLSL compiler
    return ShaderCompiler(std::move(code));
}

//---------------------------------------------------
//: Create a descriptor layout through shader reflection
//---------------------------------------------------
std::map<ui32, std::vector<IDescriptorLayoutBinding>> Shader::getDescriptorLayoutBindings(const ShaderModule &module) {
    std::map<ui32, std::vector<IDescriptorLayoutBinding>> bindings{};
    
    //: Create a SPIRV-cross compiler and obtain the shader resources (each descriptor)
    ShaderCompiler compiler = Shader::getCompiler(module.code);
    ShaderResources resources = compiler.get_shader_resources();
    
    //: Helper function to get the specific resources from the compiler that match a descriptor type
    auto get_resources = [&](ShaderDescriptorType type) {
        switch (type) {
            case DESCRIPTOR_UNIFORM:
                return &resources.uniform_buffers;
            case DESCRIPTOR_STORAGE:
                return &resources.storage_buffers;
            case DESCRIPTOR_IMAGE_SAMPLER:
                return &resources.sampled_images;
            case DESCRIPTOR_INPUT_ATTACHMENT:
                return &resources.subpass_inputs;
        }
    };
    
    //: Iterate through all descriptor types
    for (auto &[descriptor_type, name] : descriptor_names) {
        for (const auto &res : *get_resources(descriptor_type)) {
            //---Decorations---
            
            //: Binding, indicated by layout(binding = 0), necessary for IDescriptorLayoutBinding creation
            ui32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        
            //: Descriptor set, indicated by layout(set = 0), for multiple descriptor set support
            ui32 set = 0;
            if (compiler.has_decoration(res.id, spv::DecorationDescriptorSet))
                set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            
            //: Create a layout binding
            IDescriptorLayoutBinding data;
            data.binding = binding;
            data.descriptorType = IF_VULKAN((VkDescriptorType)) descriptor_type;
            data.descriptorCount = 1;
            data.stageFlags = module.stage;
            data.pImmutableSamplers = nullptr;
            
            //: Add it to the binding list
            bindings[set].push_back(data);
            log::graphics(" - %s (%s) - Set: %d - Binding : %d - Stage: %s",
                          name.c_str(), res.name.c_str(), set, binding, shader_stage_names.at(module.stage).c_str());
        }
    }
    
    return bindings;
}

//---------------------------------------------------
//: Get the compute x,y,z group size from a SPIRV shader
//---------------------------------------------------
std::array<ui32, 3> Shader::getComputeGroupSize(const ShaderPass &pass) {
    //: Get the compute module for the ShaderPass
    const ShaderModule* compute_module = Shader::getPassModule(pass, SHADER_STAGE_COMPUTE);
    
    //: Prepare a SPIRV-cross compiler
    ShaderCompiler compiler = Shader::getCompiler(compute_module->code);
    
    //: Obtain the group size as specialization constants
    std::vector<spirv_cross::SpecializationConstant> a(3);
    ui32 constant = compiler.get_work_group_size_specialization_constants(a[0], a[1], a[2]);
    auto &c = compiler.get_constant(constant);
    
    //: Get the raw group size data from the constants
    std::array<ui32, 3> sizes = { c.m.c[0].r[0].u32, c.m.c[0].r[1].u32, c.m.c[0].r[2].u32 };
    
    return sizes;
}
