//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_api.h"

#include <fstream>
#include <vector>
#include <filesystem>

#include "log.h"

using namespace Verse;
using namespace Graphics;

//Common API calls for Vulkan and OpenGL

std::vector<char> API::readSPIRV(std::string filename) {
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

ShaderCode API::readSPIRV(const ShaderLocations &locations) {
    ShaderCode code;
    
    if (locations.vert.has_value())
        code.vert = readSPIRV(locations.vert.value());
    if (locations.frag.has_value())
        code.frag = readSPIRV(locations.frag.value());
    if (locations.compute.has_value())
        code.compute = readSPIRV(locations.compute.value());
    if (locations.geometry.has_value())
        code.geometry = readSPIRV(locations.geometry.value());
    
    return code;
}

ShaderData API::createShaderData(str name) {
    //---Shader data---
    //      Creates a shader data object from a list of locations for the different stages
    //      First it saves the locations, then it reads the code, and then gets the stage create info
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
    
    data.code = API::readSPIRV(data.locations);
    
    return data;
}

ShaderCompiler API::getShaderCompiler(const std::vector<char> &code) {
    std::vector<ui32> spirv;
    
    for (int i = 0; i < code.size() / 4; i++) {
        spirv.push_back((code[4*i] << 24) |
                        (code[4*i+1] << 16) |
                        (code[4*i+2] << 8) |
                         code[4*i+3]);
    }
    
    return spirv_cross::CompilerGLSL(std::move(spirv));
}
