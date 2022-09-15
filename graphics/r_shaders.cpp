//* shaders
//      different utilities relating to rendering shaders

#include "r_shaders.h"

#include "file.h"
#include "fresa_assert.h"

using namespace fresa;

//* get shader source from file
std::vector<ui32> graphics::shader::readSPIRV(str name, ShaderStage type) {
    //: calculate the path to the spirv file
    str path = file::path(fmt::format("shaders/{}/{}.{}.spv", name, name, shader_extensions.at((ui32)type)));

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