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
spv_c::CompilerGLSL shader::createCompiler(std::vector<ui32> code) {
    //: revert the bytes in each ui32 (necessary for spirv-cross)
    for (auto i : code) {
        ui8 *istart = (ui8*)&i, *iend = (ui8*)&i + sizeof(ui32) / sizeof(ui8);
        std::reverse(istart, iend);
    }

    //: create the compiler
    return spv_c::CompilerGLSL(std::move(code));
}

//* get descriptor layout bindings
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

// ··················
// · VULKAN SHADERS ·
// ··················

//* create vulkan shader representation
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