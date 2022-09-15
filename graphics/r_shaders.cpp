//* shaders
//      different utilities relating to rendering shaders

#include "r_shaders.h"
#include "r_api.h"

#include "file.h"
#include "fresa_assert.h"

using namespace fresa;
using namespace graphics;

//* get shader source from file
std::vector<ui32> shader::readSPIRV(str name, ShaderStages stage) {
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

//* create shader module
VkShaderModule shader::createModule(str name, ShaderStages stage) {
    soft_assert(api != nullptr, "the graphics api is not initialized");

    //: read the spirv code
    auto code = readSPIRV(name, stage);

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