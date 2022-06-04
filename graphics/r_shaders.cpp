//: fresa by jose pazos perez, licensed under GPLv3
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
    module.module = Shader::API::createInternalShaderModule(module.code, module.stage);
    
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
    
    //---Descriptor sets---
    
    pass.descriptors = createDescriptorSets(pass.stages);
    
    //: Gather all layouts into one vector for pipeline layout creation
    std::vector<IDescriptorLayout> descriptor_layouts;
    for (auto &d : pass.descriptors) {
        descriptor_layouts.resize(d.set + 1);
        descriptor_layouts.at(d.set) = d.layout;
    }
    
    //---Pipeline layout (Vulkan)---
    
    IF_VULKAN(pass.pipeline_layout = VK::createPipelineLayout(descriptor_layouts, {});)
    
    //---Pipeline---
    
    //: Check if it is a draw or compute shader
    bool draw_shader = false, compute_shader = false;
    for (auto &s : pass.stages) {
        if (s.stage == SHADER_STAGE_VERTEX or s.stage == SHADER_STAGE_FRAGMENT)
            draw_shader = true;
        if (s.stage == SHADER_STAGE_COMPUTE)
            compute_shader = true;
    }
    if (draw_shader and compute_shader)
        log::error("You can't have vert/frag stages along with a compute stage in the same shaderpass");
    
    return pass;
}

//---------------------------------------------------
//: Register shader
//---------------------------------------------------
void Shader::registerShader(str name, ShaderType type) {
    //: ShaderID
    ShaderID id{name};
    
    if (shaders.types.count(id))
        log::error("You are adding a repeated shader, %s", name.c_str());
    if (type == SHADER_LAST)
        log::error("Invalid shader type");
    
    //: Add to type list
    shaders.types[id] = type;
    //: Add to shader list
    shaders.list[type][id] = Shader::createPass(name);
}

//---------------------------------------------------
//: Get a shader by its id
//---------------------------------------------------
const ShaderPass& Shader::getShader(ShaderID shader) {
    if (not shaders.types.count(shader))
        log::error("The shader you specified is not valid (%s)", shader.c_str());
    ShaderType type = shaders.types.at(shader);
    return shaders.list.at(type).at(shader);
}

//---------------------------------------------------
//: Get a specific stage module from a ShaderPass
//---------------------------------------------------
const ShaderModule& Shader::getPassModule(const ShaderPass &pass, ShaderStage stage) {
    //: Find module that matches the shader stage
    for (auto &m : pass.stages) {
        if (m.stage != stage)
            continue;
        return m;
    }
    
    //: Error checking
    log::error("You are trying to get a %s ShaderStage that is not present in this ShaderPass", shader_extensions.at(stage).c_str()); throw 0;
}

//---------------------------------------------------
//: Get a SPIRV-cross compiler for a specific shader
//      It has to revert all the bits in the shader code since SPIRV-cross takes it that way
//      Important for shader reflection and creating descriptor sets automatically
//---------------------------------------------------
spv_c::CompilerGLSL Shader::getCompiler(std::vector<ui32> code) {
    //: Revert the bytes in each ui32
    for (auto i : code) {
        ui8* istart = (ui8*)&i, *iend = (ui8*)&i + sizeof(ui32) / sizeof(ui8);
        std::reverse(istart, iend);
    }
    
    //: Create a SPIRV-cross GLSL compiler
    return spv_c::CompilerGLSL(std::move(code));
}

//---------------------------------------------------
//: Create a descriptor layout through shader reflection
//---------------------------------------------------
std::map<ui32, std::vector<IDescriptorLayoutBinding>> Shader::getDescriptorLayoutBindings(const ShaderModule &module) {
    std::map<ui32, std::vector<IDescriptorLayoutBinding>> bindings{};
    
    //: Create a SPIRV-cross compiler and obtain the shader resources (each descriptor)
    spv_c::CompilerGLSL compiler = Shader::getCompiler(module.code);
    spv_c::ShaderResources resources = compiler.get_shader_resources();
    
    //: Helper function to get the specific resources from the compiler that match a descriptor type
    auto get_resources = [&](ShaderDescriptor type) {
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
            
            //: Create a layout binding
            IDescriptorLayoutBinding data;
            
            //: Descriptor set, indicated by layout(set = 0), for multiple descriptor set support
            ui32 set = 0;
            if (compiler.has_decoration(res.id, spv::DecorationDescriptorSet))
                set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            
            //: Binding, indicated by layout(binding = 0), necessary for IDescriptorLayoutBinding creation
            data.binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        
            //: Buffer size, only for uniform and storage, used to allocate the descriptor buffers
            if (descriptor_type == DESCRIPTOR_UNIFORM or descriptor_type == DESCRIPTOR_STORAGE) {
                const spv_c::SPIRType &type = compiler.get_type(res.base_type_id);
                data.size = (ui32)compiler.get_declared_struct_size(type);
                if (data.size == 0) {
                    constexpr ui32 MAX_OBJECTS = 10000;
                    data.size = (ui32)compiler.get_declared_struct_size_runtime_array(type, MAX_OBJECTS);
                }
            }
            
            //: Descriptor name
            data.name = res.name;
            
            //: Other properties
            data.descriptor_type = descriptor_type;
            data.descriptor_count = 1;
            data.stage_flags = module.stage;
            
            //: Add it to the binding list
            bindings[set].push_back(data);
            log::graphics(" - %s (%s) - Set: %d - Binding : %d - Stage: %s",
                          name.c_str(), data.name.c_str(), set, data.binding, shader_stage_names.at(module.stage).c_str());
        }
    }
    
    return bindings;
}

//---------------------------------------------------
//: Get the compute x,y,z group size from a SPIRV shader
//---------------------------------------------------
std::array<ui32, 3> Shader::getComputeGroupSize(const ShaderPass &pass) {
    //: Get the compute module for the ShaderPass
    const ShaderModule& compute_module = Shader::getPassModule(pass, SHADER_STAGE_COMPUTE);
    
    //: Prepare a SPIRV-cross compiler
    spv_c::CompilerGLSL compiler = Shader::getCompiler(compute_module.code);
    
    //: Obtain the group size as specialization constants
    std::vector<spv_c::SpecializationConstant> a(3);
    ui32 constant = compiler.get_work_group_size_specialization_constants(a[0], a[1], a[2]);
    auto &c = compiler.get_constant(constant);
    
    //: Get the raw group size data from the constants
    std::array<ui32, 3> sizes = { c.m.c[0].r[0].u32, c.m.c[0].r[1].u32, c.m.c[0].r[2].u32 };
    
    return sizes;
}

//---------------------------------------------------
//: Create descriptor sets
//---------------------------------------------------
std::vector<DescriptorSet> Shader::createDescriptorSets(const std::vector<ShaderModule> &stages) {
    std::vector<DescriptorSet> data{};
    
    //: Create a descriptor pool if there are none
    if (descriptor_pools.size() == 0)
        descriptor_pools.push_back(Shader::API::createDescriptorPool(descriptor_pool_sizes));
    
    //: Get bindings for all stages
    std::map<ui32, std::vector<IDescriptorLayoutBinding>> bindings{};
    for (auto &stage : stages) {
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
                if (b.at(j).descriptor_type != b.at(i).descriptor_type)
                    log::error("Descriptor types for the same binding (%d) must be the same, and they are %s and %s", binding,
                               descriptor_names.at((ShaderDescriptor)b.at(j).descriptor_type).c_str(),
                               descriptor_names.at((ShaderDescriptor)b.at(j).descriptor_type).c_str());
                if (b.at(j).stage_flags == b.at(i).stage_flags)
                    log::error("It is not allowed to repeat the same binding (%d) in the same stage (%s)", binding,
                               shader_stage_names.at((ShaderStage)b.at(j).stage_flags).c_str());
                
                //: Merge stage flags
                b.at(j).stage_flags = ShaderStage(b.at(j).stage_flags | b.at(i).stage_flags);
            }
            //: New binding
            else {
                binding_to_index[binding] = i;
            }
        }
        
        //: Add new set
        data.push_back(DescriptorSet{});
        DescriptorSet &current_set = data.back();
        current_set.set = set;
        
        //: Sort and create final bindings
        for (auto &[_, i] : binding_to_index)
            current_set.bindings.push_back(b.at(i));
        
        //: Create descriptor layout
        IF_VULKAN(
            std::vector<VkDescriptorSetLayoutBinding> bindings{};
            for (auto &b : current_set.bindings)
                bindings.push_back(VkDescriptorSetLayoutBinding{
                    b.binding,
                    (ShaderDescriptorT)b.descriptor_type,
                    b.descriptor_count,
                    b.stage_flags,
                    nullptr});
            current_set.layout = VK::createDescriptorSetLayout(bindings);
        )
        IF_OPENGL(current_set.layout = current_set.bindings;)
        
        //: Allocate descriptor sets
        std::vector<IDescriptorSet> descriptors = Shader::API::allocateDescriptorSets(current_set.layout, &descriptor_pools.back());
        if (descriptors.size() == 0) { //: Create new descriptor pool and retry allocation
            descriptor_pools.push_back(Shader::API::createDescriptorPool(descriptor_pool_sizes));
            descriptors = Shader::API::allocateDescriptorSets(current_set.layout, &descriptor_pools.back());
            if (descriptors.size() == 0)
                log::error("Couldn't allocate this descriptor set");
        }
        
        //: Save to set
        current_set.pool = &descriptor_pools.back();
        for (int j = 0; j < Config::frames_in_flight; j++)
            current_set.descriptors[j] = descriptors.at(j);
        
        //: Create resources
        current_set.resources = createDescriptorResources(current_set.bindings);
    }
    
    return data;
}

//---------------------------------------------------
//: Create descriptor resources (uniforms, textures...)
//---------------------------------------------------

std::vector<ShaderResource> Shader::createDescriptorResources(const std::vector<IDescriptorLayoutBinding> &bindings) {
    std::vector<ShaderResource> resources{};
    
    for (auto &b : bindings) {
        if (b.descriptor_type == DESCRIPTOR_INPUT_ATTACHMENT)
            continue;
        
        ShaderResource res{};
        res.type = b.descriptor_type;
        res.name = b.name;
        
        if (b.descriptor_type == DESCRIPTOR_UNIFORM)
            res.id = Buffer::registerUniformBuffer(b.name, b.size);
        
        if (b.descriptor_type == DESCRIPTOR_STORAGE)
            res.id = Buffer::registerStorageBuffer(b.name, b.size);
        
        if (b.descriptor_type == DESCRIPTOR_IMAGE_SAMPLER) {
            //log::info("Image descriptors not supported yet");
            continue;
        }
        
        resources.push_back(res);
    }
    
    return resources;
}
