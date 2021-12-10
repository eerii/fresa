//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#include "r_api.h"

#include <fstream>
#include <filesystem>

#include "log.h"

using namespace Fresa;
using namespace Graphics;

//---Common API calls for Vulkan and OpenGL---

std::vector<char> API::readSPIRV(std::string filename) {
    //---Read SPIRV---
    //      Opens a SPIRV shader file and returns an array with the data
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        log::error("Failed to open the shader file (%s)", filename.c_str());
    
    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);
    
    file.seekg(0);
    file.read(buffer.data(), file_size);
    
    file.close();

    return buffer;
}

ShaderData API::createShaderData(str name) {
    //---Shader data---
    //      Creates a shader data object from a list of locations for the different stages
    //      First it saves the locations and then it reads the SPIRV code
    ShaderData data;
    
    std::filesystem::path vert_location{"res/shaders/" + name + "/" + name + ".vert.spv"};
    std::filesystem::path frag_location{"res/shaders/" + name + "/" + name + ".frag.spv"};
    std::filesystem::path compute_location{"res/shaders/" + name + "/" + name + ".compute.spv"};
    std::filesystem::path geometry_location{"res/shaders/" + name + "/" + name + ".geometry.spv"};
    
    if (std::filesystem::exists(vert_location))
        data.locations.vert = vert_location.string();
    if (std::filesystem::exists(frag_location))
        data.locations.frag = frag_location.string();
    if (std::filesystem::exists(compute_location))
        data.locations.compute = compute_location.string();
    if (std::filesystem::exists(geometry_location))
        data.locations.geometry = geometry_location.string();
    
    if (data.locations.vert.has_value())
        data.code.vert = readSPIRV(data.locations.vert.value());
    if (data.locations.frag.has_value())
        data.code.frag = readSPIRV(data.locations.frag.value());
    if (data.locations.compute.has_value())
        data.code.compute = readSPIRV(data.locations.compute.value());
    if (data.locations.geometry.has_value())
        data.code.geometry = readSPIRV(data.locations.geometry.value());
    
    return data;
}

ShaderCompiler API::getShaderCompiler(const std::vector<char> &code) {
    //---Shader compiler---
    //      Creates and returns the SPIR-V Cross compiler for the shader code
    //      This for some reason requires to to invert the code bits, so that is done before passing to the CompilerGLSL function
    std::vector<ui32> spirv;
    
    for (int i = 0; i < code.size() / 4; i++) {
        spirv.push_back((code[4*i] << 24) |
                        (code[4*i+1] << 16) |
                        (code[4*i+2] << 8) |
                         code[4*i+3]);
    }
    
    return spirv_cross::CompilerGLSL(std::move(spirv));
}
