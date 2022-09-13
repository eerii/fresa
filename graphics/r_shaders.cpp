//* shaders
//      different utilities relating to rendering shaders

#include "r_api.h"
#include "file.h"

using namespace fresa;

//* get shader source from file
std::vector<ui32> graphics::readSPIRV(str name, ShaderType type) {
    //: calculate the path to the spirv file
    str path = fmt::format("shaders/{}.{}", name, shader_extensions.at((ui32)type));

    //: open the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    graphics_assert<str_view>(file.is_open(), "failed to open shader file '{}'", path);

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