//* shaders
//      different utilities relating to rendering shaders

#include "r_shaders.h"
#include "r_api.h"

#include "file.h"
#include "fresa_assert.h"
#include "constexpr_for.h"

using namespace fresa;
using namespace graphics;

// ·········
// · SPIRV ·
// ·········

//* get shader source from file
//      reads the shader source from a .spv file, which must have already been compiled to spirv
//      spirv is the binary shader format which is used by vulkan
std::vector<ui32> shader::readSPIRV(str name, ShaderStage stage) {
    //: calculate the path to the spirv file
    str path = file::path(fmt::format("shaders/{}/{}.{}.spv", name, name, shader_stages.at((ui32)stage).extension));

    //: open the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    strong_assert<str_view>(file.is_open(), "failed to open shader file '{}'", path);

    //: create a ui32 buffer (suitable for spirv-cross)
    std::size_t file_size = file.tellg();
    std::vector<ui32> code(file_size / sizeof(ui32));

    //: read the file into the buffer
    file.seekg(0);
    file.read((char*)code.data(), file_size);

    //: close and return
    file.close();
    return code;
}

//* create spirv compiler
//      a compiler is a tool spirv cross provides that can be used to get reflection data from the shader's code
//      it can also be used for cross compilation, for example, to turn spirv back to glsl for opengl
//      when cross compiling, it can also alter the shader code to make it compatible with previous versions or another api such as webgl
spv_c::CompilerGLSL shader::createCompiler(std::vector<ui32> code) {
    //: revert the bytes in each ui32 (necessary for spirv-cross)
    for (auto i : code) {
        ui8 *istart = (ui8*)&i, *iend = (ui8*)&i + sizeof(ui32) / sizeof(ui8);
        std::reverse(istart, iend);
    }

    //: create the compiler
    return spv_c::CompilerGLSL(std::move(code));
}

// ··················
// · SHADER MODULES ·
// ··················

//* create vulkan shader representation
//      a vk shader module is a vulkan object that represents the underlying shader code for one stage
VkShaderModule shader::createVkShader(const std::vector<ui32>& code) {
    soft_assert(api != nullptr, "the graphics api is not initialized");

    //: create info
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size() * sizeof(ui32);
    create_info.pCode = code.data();

    //: create the shader module
    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(api->gpu.device, &create_info, nullptr, &shader_module);
    strong_assert(result == VK_SUCCESS, "failed to create shader module");

    //: cleanup and return
    const_cast<GraphicsAPI*>(api.get())->deletion_queue_global.push([shader_module]() { vkDestroyShaderModule(api->gpu.device, shader_module, nullptr); });
    return shader_module;
}

//* create shader module object
//      not to be confused with a vk shader module, this is a wrapper type that contains the vk module, the stage it represents and the reflected bindings
ShaderModule shader::createModule(str name, ShaderStage stage) {
    ShaderModule sm;

    //: read the spirv code and create compiler
    auto code = readSPIRV(name, stage);
    auto compiler = createCompiler(code);

    //: create the vulkan shader
    sm.module = createVkShader(code);

    //: set stage
    sm.stage = stage;

    //: get descriptor layout bindings
    sm.bindings = getDescriptorBindings(compiler, stage);
    
    return sm;
}

// ···················
// · DESCRIPTOR SETS ·
// ···················

//* get descriptor layout bindings
//      to create a descriptor set we must first provide its layout, and to do that we must provide a list of bindings
//      a binding is a resource we attach to the shader, for example, an uniform buffer or a texture
//      each binding has a binding number, which must be unique inside the set, and a descriptor name
//      we use the spirv compiler to get the shader resources and process them into a list of bindings
std::vector<DescriptorLayoutBinding> shader::getDescriptorBindings(const spv_c::CompilerGLSL& compiler, ShaderStage stage) {
    //: get the reflection resources
    //      a resource is one input/output of the shader, for example a uniform buffer or a texture
    spv_c::ShaderResources resources = compiler.get_shader_resources();
    auto descriptor_types = std::tuple{
        resources.uniform_buffers,
        resources.storage_buffers,
        resources.sampled_images,
        resources.subpass_inputs
    };

    //: create the descriptor layout binding list
    std::vector<DescriptorLayoutBinding> bindings;

    for_<0, std::tuple_size_v<decltype(descriptor_types)>>([&](auto type){
        for (const auto &res : std::get<type>(descriptor_types)) {
            DescriptorLayoutBinding data;

            //: binding, indicated by layout(binding = 0), necessary for VkDescriptorSetLayoutBinding creation
            data.binding = compiler.get_decoration(res.id, spv::DecorationBinding);

            //: set, indicated by layout(set = 0), for multiple descriptor set support
            data.set = 0;
            if (compiler.has_decoration(res.id, spv::DecorationDescriptorSet))
                data.set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            
            //: buffer size, only for uniform and storage, used to allocate the descriptor buffers
            //      - todo: calculate runtime size for variable sized arrays inside buffers
            if (type.value == (int)ShaderDescriptor::UNIFORM || type.value == (int)ShaderDescriptor::STORAGE) {
                const spv_c::SPIRType &spv_t = compiler.get_type(res.base_type_id);
                data.size = (ui32)compiler.get_declared_struct_size(spv_t);
                if (data.size == 0) {
                    log::warn("this shader includes a runtime array, which is not fully supported yet");
                    data.size = (ui32)compiler.get_declared_struct_size_runtime_array(spv_t, 1024);
                }
            }

            //: descriptor name
            data.name = res.name;

            //: other properties
            data.descriptor_type = shader_descriptors.at(type.value).descriptor_type;
            data.descriptor_count = 1;
            data.stage_flags = stage;

            //: add to the list
            bindings.push_back(data);
        }
    });

    //: return the bindings
    return bindings;
}

//* create descriptor set layout
//      once we have the reflected bindings, we can encapsule them in the vulkan descriptor layout object
//      for that, we first group the bindings by set, and join bindings in multiple stages on a same entry
//      then, we return a map of the set number to the associated layout object, to be used for descriptor set creation
std::unordered_map<ui32, VkDescriptorSetLayout> shader::createDescriptorLayout(const std::vector<ShaderModule> &stages) {
    soft_assert(api != nullptr, "the graphics api is not initialized");

    //: we are going to order the bindings by set, merging bindings from each stage, and transforming them into vulkan structures
    std::unordered_map<ui32, std::vector<VkDescriptorSetLayoutBinding>> set_bindings;
    for (const auto &stage : stages) {
        for (const auto &binding : stage.bindings) {
            auto &s = set_bindings[binding.set];
            auto vk_stage_flag = shader_stages.at((ui32)binding.stage_flags).stage;
            //: check if there is already a binding with the same number
            auto it = std::find_if(s.begin(), s.end(), [&](const auto &b){ return b.binding == binding.binding; });
            if (it != s.end()) {
                //: if the binding already exists, check for errors and then merge the stage flags
                strong_assert<const ui32, const str_view, const str_view>(it->descriptorType == binding.descriptor_type,
                              "descriptor types for the same binding ({}) must be the same, but are '{}' and '{}'",
                              std::forward<const ui32>(binding.binding), std::forward<const str_view>(shader_descriptors.at(it->descriptorType).name), std::forward<const str_view>(shader_descriptors.at(binding.descriptor_type).name));
                strong_assert<const ui32, const str_view>(it->stageFlags == vk_stage_flag,
                              "it is not allowed to repeat the same binding ({}) in the same stage '{}'",
                              std::forward<const ui32>(binding.binding), std::forward<const str_view>(shader_descriptors.at(binding.descriptor_type).name));
                it->stageFlags |= vk_stage_flag;
            } else {
                //: if the binding does not exist, create it
                s.push_back(VkDescriptorSetLayoutBinding{
                    .binding = binding.binding,
                    .descriptorType = binding.descriptor_type,
                    .descriptorCount = binding.descriptor_count,
                    .stageFlags = vk_stage_flag
                });
            }
        }
    }

    //: create the layouts
    std::unordered_map<ui32, VkDescriptorSetLayout> layouts;
    for (const auto &[set, bindings] : set_bindings) {
        VkDescriptorSetLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = (ui32)bindings.size(),
            .pBindings = bindings.data()
        };
        layouts[set] = VkDescriptorSetLayout{};
        VkResult result = vkCreateDescriptorSetLayout(api->gpu.device, &layout_info, nullptr, &layouts.at(set));
        strong_assert(result == VK_SUCCESS, "failed to create a descriptor set layout");
    }

    //: cleanup and return
    const_cast<GraphicsAPI*>(api.get())->deletion_queue_global.push([layouts]() {
        for (const auto &[_, layout] : layouts)
            vkDestroyDescriptorSetLayout(api->gpu.device, layout, nullptr);
    });
    return layouts;
}

//* create descriptor pool
//      a descriptor pool will house descriptor sets of various kinds
//      there are two types of usage for a descriptor pool:
//      + allocate sets once at the start of the program, and then use them each time
//        this is what we are doing here, so we can know the exact pool size and destroy the pool at the end
//      + allocate sets per frame, this can be implemented in the future
//        tt can be cheap using VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT and resetting the entire pool per frame
//        we would have a list of descriptor pools with big sizes for each type of descriptor, and if an allocation fails,
//        just create another pool and add it to the list. at the end of the frame all of them get deleted.
VkDescriptorPool shader::createDescriptorPool() {
    //: pool sizes
    //      we need to define how many descriptors of each type a pool can host
    //      the propper way to do this would be to average the number of descriptors on each shader and get the relative frequencies
    //      also, they can be grouped by descriptor layouts, so it can even waste less memory
    //      however, in the interest of keeping everything simple for now, we just allocate descriptor_pool_max_sets of each type we are using
    constexpr auto pool_sizes = std::to_array<VkDescriptorPoolSize>({
        { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            .descriptorCount = 1 * engine_config.vk_descriptor_pool_max_sets() },
        { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,            .descriptorCount = 1 * engine_config.vk_descriptor_pool_max_sets() },
        { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    .descriptorCount = 1 * engine_config.vk_descriptor_pool_max_sets() },
        { .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,          .descriptorCount = 1 * engine_config.vk_descriptor_pool_max_sets() },
    });

    //: create info
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = engine_config.vk_descriptor_pool_max_sets(),
        .poolSizeCount = (ui32)pool_sizes.size(),
        .pPoolSizes = pool_sizes.data()
    };

    //: create descriptor pool
    VkDescriptorPool pool;
    VkResult result = vkCreateDescriptorPool(api->gpu.device, &pool_info, nullptr, &pool);
    strong_assert(result == VK_SUCCESS, "failed to create a descriptor pool");

    //: cleanup and return
    const_cast<GraphicsAPI*>(api.get())->deletion_queue_global.push([pool]() { vkDestroyDescriptorPool(api->gpu.device, pool, nullptr); });
    return pool;
}