//* shaders
//      different utilities relating to rendering shaders

#include "r_shaders.h"

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
std::vector<ui32> shader::readSPIRV(str_view name, ShaderStage stage) {
    //: calculate the path to the spirv file
    str path = file::path(fmt::format("shaders/{}/{}.{}.spv", name, name, shader_stage_extensions.at((ui32)stage)));

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

            //: binding, indicated by layout(binding = 0), necessary for vkDescriptorSetLayoutBinding creation
            data.binding = compiler.get_decoration(res.id, spv::DecorationBinding);

            //: set, indicated by layout(set = 0), for multiple descriptor set support
            data.set = 0;
            if (compiler.has_decoration(res.id, spv::DecorationDescriptorSet))
                data.set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            
            //: buffer size, only for uniform and storage, used to allocate the descriptor buffers
            //      - todo: calculate runtime size for variable sized arrays inside buffers
            if (type.value == ShaderDescriptor::UNIFORM or type.value == ShaderDescriptor::STORAGE) {
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
            data.descriptor_type = (ShaderDescriptor)type.value;
            data.descriptor_count = 1;
            data.stage_flags = stage;

            //: add to the list
            bindings.push_back(data);
        }
    });

    //: return the bindings
    return bindings;
}