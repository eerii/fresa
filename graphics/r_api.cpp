//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_api.h"

#include <fstream>

#include "file.h"
#include "log.h"
#include "config.h"

//: SDL Window Flags
#if defined USE_VULKAN
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    #define RENDERER_NAME "Vulkan"
#elif defined USE_OPENGL
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    #define RENDERER_NAME "OpenGL"
#endif

using namespace Fresa;
using namespace Graphics;

//---Common API calls for Vulkan and OpenGL---

WindowData API::createWindow(Vec2<ui32> size, str name) {
    //---Create window data---
    WindowData win;
    
    //: SDL window
#ifdef __EMSCRIPTEN__
    SDL_Renderer* renderer = nullptr;
    SDL_CreateWindowAndRenderer(size.x, size.y, SDL_WINDOW_OPENGL, &win.window, &renderer);
    
    if (win.window == nullptr or renderer == nullptr)
        log::error("Failed to create a Window and a Renderer", SDL_GetError());
#else
    win.window = SDL_CreateWindow((name + " - " + RENDERER_NAME).c_str(),
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, W_FLAGS);
    
    if (win.window == nullptr)
        log::error("Failed to create a Window", SDL_GetError());
    
    SDL_SetWindowResizable(win.window, SDL_TRUE);
    SDL_SetWindowMinimumSize(win.window, 256, 180);
#endif
    
    //: Window size
    win.size = size;
    
    //: Refresh rate
    win.refresh_rate = getRefreshRate(win);
    log::graphics("Refresh Rate: %d", win.refresh_rate);
    
    //: Calculate resolution and scale
    Vec2<float> ratios = win.size.to<float>() / Config::resolution.to<float>();
    win.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //: V-Sync (it only changes something for OpenGL at the moment)
    win.vsync = true;
    
    return win;
}

ui16 API::getRefreshRate(WindowData &win, bool force) {
    //---Refresh rate---
    //      Either returns the already calculated refresh rate or it gets it using SDL_DisplayMode
    if (win.refresh_rate > 0 and not force)
        return win.refresh_rate;
    
    int displayIndex = SDL_GetWindowDisplayIndex(win.window);
    
    SDL_DisplayMode mode;
    if(SDL_GetDisplayMode(displayIndex, 0, &mode))
        log::error("Error getting display mode: ", SDL_GetError());
    
    return (ui16)mode.refresh_rate;
}

ui32 relative_subpass(RenderPassID r_id, SubpassID s_id) {
    auto subpass_list = getBimapAtoB<SubpassID>(r_id, renderpass_subpass);
    auto it = std::find(subpass_list.begin(), subpass_list.end(), s_id);
    if (it == subpass_list.end()) {
        log::error("The subpass %d is not part of the render pass %d", s_id, r_id);
        return 0;
    }
    return (ui32)(it - subpass_list.begin());
}

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
    
    data.locations.vert = File::path_optional("shaders/" + name + "/" + name + ".vert.spv");
    data.locations.frag = File::path_optional("shaders/" + name + "/" + name + ".frag.spv");
    data.locations.compute = File::path_optional("shaders/" + name + "/" + name + ".compute.spv");
    data.locations.geometry = File::path_optional("shaders/" + name + "/" + name + ".geometry.spv");
    
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
