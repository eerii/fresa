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
    #include "r_vulkan_api.h"
#elif defined USE_OPENGL
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    #define RENDERER_NAME "OpenGL"
    #include "r_opengl_api.h"
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
    
    data.locations.vert = File::path_optional("render/" + name + "/" + name + ".vert.spv");
    data.locations.frag = File::path_optional("render/" + name + "/" + name + ".frag.spv");
    data.locations.compute = File::path_optional("render/" + name + "/" + name + ".compute.spv");
    data.locations.geometry = File::path_optional("render/" + name + "/" + name + ".geometry.spv");
    
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

void API::processRendererDescription(GraphicsAPI &api, const WindowData &win) {
    if (API::renderer_description_path.size() == 0)
        log::error("You need to set API::renderer_description_path with the location of your renderer description file");
    
    if (not std::filesystem::exists(std::filesystem::path{API::renderer_description_path}))
        log::error("Error creating the rederer description. Please make sure that you have created an appropiate file at '%s'. The renderer description is a list of the attachments, subpasses, renderpasses and pipelines/shaders in your rendering application. It needs to be filled accordingly. There is an example of a valid file in https://github.com/josekoalas/aguacate", API::renderer_description_path.c_str());
    
    std::map<str, AttachmentID> attachment_list{};
    std::map<str, SubpassID> subpass_list{};
    int swapchain_count = 0; //: Support for multiple swapchain attachments
    
    std::ifstream f(API::renderer_description_path);
    std::string s;
    while (std::getline(f, s)) {
        if (s.size() == 0)
            continue;
        
        if (s.at(0) == 'a') {
            std::vector<str> a = split(s); //: a name attachment_type resolution
            
            //: Name
            str name = a.at(1);
            if (name == "swapchain") {
                attachment_list[name + std::to_string(++swapchain_count)] = API::registerAttachment(api, ATTACHMENT_COLOR_SWAPCHAIN, win.size);
                continue;
            }
            if (attachment_list.count(name))
                log::error("Duplicated attachment name %s", name.c_str());
            if (a.size() != 4)
                log::error("You have not provided all the required parameters for an attachment");
            
            //: Attachment type
            std::vector<str> type_str = split(a.at(2), "_");
            if (type_str.size() == 0)
                log::error("You must provide an attachment type for attachments other than swapchain");
            if (not attachment_type_names.count(type_str.at(0)))
                log::error("You provided an invalid attachment type, check the name list for all the options, index 0");
            AttachmentType type = attachment_type_names.at(type_str.at(0));
            for (int i = 1; i < type_str.size(); i++) {
                if (not attachment_type_names.count(type_str.at(i)))
                    log::error("You provided an invalid attachment type, check the name list for all the options, index %d", i);
                type = (AttachmentType)(type | attachment_type_names.at(type_str.at(i)));
            }
            attachment_list[a.at(1)] = type;
            
            //: Resolution
            Vec2<> resolution{};
            if (a.at(3) == "res") {
                resolution = Config::resolution.to<int>();
            }
            else if (a.at(3) == "win") {
                resolution = win.size;
            }
            else {
                std::vector<str> res_str = split(a.at(3), "x");
                if (not (res_str.size() == 2))
                    log::error("You need to either provide an smart attachment resolution (win, res...) or a numeric value in the form 1920x1080");
                resolution = Vec2<>(std::stoi(res_str.at(0)), std::stoi(res_str.at(1)));
            }
            
            //: Register attachment
            attachment_list[name] = API::registerAttachment(api, type, resolution);
        }
        
        if (s.at(0) == 's') {
            std::vector<str> s1 = split(s, "["); //: s name [a1 a2 a3] [ext1 ext2]
            if (s1.size() < 2)
                log::error("You need to provide at least an attachment list for the subpass, for example 's name [a1 a2]'");
            if (s1.size() > 3)
                log::error("There are too many arguments for this subpass");
            
            //: Name
            str name = s1.at(0).substr(2); name.pop_back();
            if (subpass_list.count(name))
                log::error("Duplicated subpass name %s", name.c_str());
            
            //: Attachments
            std::vector<AttachmentID> subpass_attachments{};
            std::vector<str> s2 = split(s1.at(1).substr(0, s1.at(1).find("]")));
            for (auto a : s2) {
                if (a == "swapchain")
                    a += std::to_string(swapchain_count);
                if (not attachment_list.count(a))
                    log::error("You have used an incorrect attachment name, %s", a.c_str());
                subpass_attachments.push_back(attachment_list.at(a));
            }
            
            //: External attachments
            std::vector<AttachmentID> external_attachments{};
            if (s1.size() == 3) {
                std::vector<str> s3 = split(s1.at(2).substr(0, s1.at(2).find("]")));
                for (auto &a : s3) {
                    if (not attachment_list.count(a))
                        log::error("You have used an incorrect external attachment name, %s", a.c_str());
                    external_attachments.push_back(attachment_list.at(a));
                }
            }
            
            //: Register subpass
            subpass_list[name] = API::registerSubpass(subpass_attachments, external_attachments);
        }
        
        #if defined USE_VULKAN
        if (s.at(0) == 'r') {
            std::vector<str> r1 = split(s, "["); //: r [s1 s2 s3]
            if (r1.size() != 2)
                log::error("The description of the renderpass is invalid, it has to be 'r [s1 s2]'");
            
            //: Subpasses
            std::vector<SubpassID> renderpass_subpasses{};
            std::vector<str> r2 = split(r1.at(1).substr(0, r1.at(1).find("]")));
            for (auto &sp : r2) {
                if (not subpass_list.count(sp))
                    log::error("You have used an incorrect subpass name, %s", sp.c_str());
                renderpass_subpasses.push_back(subpass_list.at(sp));
            }
            
            //: Register renderpass
            API::registerRenderPass(api, renderpass_subpasses);
        }
        #endif
        
        if (s.at(0) == 'p') {
            std::vector<str> p = split(s); //: p shader subpass vertexdata
            if (p.size() != 4)
                log::error("The description of the pipeline is invalid, it has to be 'p shader subpass vertexdata'");
            
            //: Shader
            Shaders shader = [p](){
                for (const auto &[sh, name] : shader_names) {
                    if (p.at(1) == name)
                        return sh;
                }
                log::error("The shader name provided (%s) is not valid", p.at(1).c_str());
                return Shaders{};
            }();
            
            //: Subpass
            if (not subpass_list.count(p.at(2)))
                log::error("You have used an incorrect subpass name, %s", p.at(2).c_str());
            SubpassID subpass = subpass_list.at(p.at(2));
            
            //: Register pipeline
            #if defined USE_VULKAN
                if (p.at(3) == "vd_color")
                    api.pipelines[shader] = VK::createPipeline<VertexDataColor>(api, shader, subpass);
                if (p.at(3) == "vd_tex")
                    api.pipelines[shader] = VK::createPipeline<VertexDataTexture>(api, shader, subpass);
                if (p.at(3) == "vd_win")
                    api.pipelines[shader] = VK::createPipeline<VertexDataWindow>(api, shader, subpass);
            #elif defined USE_OPENGL
                api.shaders[shader] = GL::createShaderDataGL(shader, subpass);
            #endif
        }
    }
}
