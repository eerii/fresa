//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_shader.h"

#include <fstream>
#include <vector>

#include "log.h"

using namespace Verse;
using namespace Graphics;

std::vector<char> Shader::readSPIRV(std::string filename) {
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

ShaderCode Shader::readSPIRV(const ShaderLocations &locations) {
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

ShaderData Shader::createShaderData(str vert, str frag, str compute, str geometry) {
    //---Shader data---
    //      Creates a shader data object from a list of locations for the different stages
    //      First it saves the locations, then it reads the code, and then gets the stage create info
    ShaderData data;
    
    if (vert != "" and not data.locations.vert.has_value())
        data.locations.vert = vert;
    if (frag != "" and not data.locations.frag.has_value())
        data.locations.frag = frag;
    if (compute != "" and not data.locations.compute.has_value())
        data.locations.compute = compute;
    if (geometry != "" and not data.locations.geometry.has_value())
        data.locations.geometry = geometry;
    
    data.code = Shader::readSPIRV(data.locations);
    
    return data;
}

ShaderCompiler Shader::getShaderCompiler(const std::vector<char> &code) {
    std::vector<ui32> spirv;
    
    for (int i = 0; i < code.size() / 4; i++) {
        spirv.push_back((code[4*i] << 24) |
                        (code[4*i+1] << 16) |
                        (code[4*i+2] << 8) |
                         code[4*i+3]);
    }
    
    return spirv_cross::CompilerGLSL(std::move(spirv));
}
