//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "r_api.h"
#include "r_window.h"
#include "r_shaders.h"

#include <fstream>

#include "file.h"
#include "log.h"
#include "config.h"

//: SDL Window Flags
#if defined USE_VULKAN
    #include "r_vulkan_api.h"
#elif defined USE_OPENGL
    #include "r_opengl_api.h"
#endif

using namespace Fresa;

//---Common API calls for Vulkan and OpenGL---

bool Graphics::hasMultisampling(AttachmentID a, bool check_samples) {
    if (not attachments.count(a))
        return false;
    AttachmentData &data = attachments.at(a);
    bool msaa = data.type & ATTACHMENT_MSAA and data.type & ATTACHMENT_COLOR;
    #ifdef USE_VULKAN
    if (check_samples)
        msaa = msaa and data.description.samples != VK_SAMPLE_COUNT_1_BIT;
    #endif
    if (msaa and attachments.size() > a + 1 and (not attachments.count(a + 1) or attachments.at(a + 1).type & ATTACHMENT_MSAA))
        log::error("Improper formatting on MSAA resolve attachment");
    return msaa;
};

void Graphics::processRendererDescription() {
    if (Config::renderer_description_path.size() == 0)
        log::error("You need to set Config::renderer_description_path with the location of your renderer description file");

    std::map<str, AttachmentID> attachment_list{};
    std::map<str, SubpassID> subpass_list{};
    int swapchain_count = 0; //: Support for multiple swapchain attachments
    
    str path = File::path(Config::renderer_description_path);
    std::ifstream f(path);
    
    std::string s;
    while (std::getline(f, s)) {
        if (s.size() == 0) continue;
        
        std::vector<str> line = split(s, " ", true, true);
        
        //---Attachment---
        //:     a name attachment_type resolution
        if (line.at(0) == "a") {
            //: Name
            str name = line.at(1);
            if (name == "swapchain") {
                attachment_list[name + std::to_string(++swapchain_count)] = registerAttachment(ATTACHMENT_COLOR_SWAPCHAIN, window.size);
                continue;
            }
            if (line.size() != 4) log::error("You have not provided all the required parameters for an attachment");
            if (attachment_list.count(name)) log::error("Duplicated attachment name %s", name.c_str());
            
            //: Attachment type
            std::vector<str> type_str = split(line.at(2), "_");
            if (type_str.size() == 0) log::error("You must provide an attachment type for attachments other than swapchain");
            AttachmentType type{};
            for (int i = 0; i < type_str.size(); i++) {
                if (not attachment_type_names.count(type_str.at(i)))
                    log::error("You provided an invalid attachment type, check the name list for all the options, index %d", i);
                type = (AttachmentType)(type | attachment_type_names.at(type_str.at(i)));
            }
            
            //: Resolution
            str res = line.at(3);
            Vec2<ui16> resolution{};
            if (res == "res") {
                resolution = Config::resolution;
            }
            else if (res == "win") {
                resolution = window.size;
                type = (AttachmentType)(type | ATTACHMENT_WINDOW);
            } else {
                std::vector<str> res_str = split(res, "x");
                if (not (res_str.size() == 2))
                    log::error("You need to either provide an smart attachment resolution (win, res...) or a numeric value in the form 1920x1080");
                resolution = Vec2<ui16>((ui16)std::stoi(res_str.at(0)), (ui16)std::stoi(res_str.at(1)));
            }
            
            //: Register attachment
            attachment_list[name] = registerAttachment(type, resolution);
            
            //: Multisampling resolve
            if (type & ATTACHMENT_MSAA and type & ATTACHMENT_COLOR)
                attachment_list[name + "_resolve"] = registerAttachment(AttachmentType(type & ~ATTACHMENT_MSAA), resolution);
        }
        
        //---Subpass---
        //:     s name [a1 a2 a3] [ext1 ext2]
        if (line.at(0) == "s") {
            if (line.size() < 3 or line.size() > 4) log::error("You have not provided all the required parameters for a subpass");
            
            //: Name
            str name = line.at(1);
            if (subpass_list.count(name)) log::error("Duplicated subpass name %s", name.c_str());
            
            //: Attachments
            std::vector<AttachmentID> subpass_attachments{};
            std::vector<str> list = split(list_contents(line.at(2)));
            for (auto a : list) {
                if (a == "swapchain") a += std::to_string(swapchain_count);
                if (not attachment_list.count(a)) log::error("You have used an incorrect attachment name, %s", a.c_str());
                subpass_attachments.push_back(attachment_list.at(a));
            }
            
            //: External attachments
            std::vector<AttachmentID> external_attachments{};
            if (line.size() == 4) {
                std::vector<str> ext_list = split(list_contents(line.at(3)));
                for (auto &a : ext_list) {
                    if (not attachment_list.count(a)) log::error("You have used an incorrect external attachment name, %s", a.c_str());
                    external_attachments.push_back(attachment_list.at(a));
                }
            }
            
            //: Register subpass
            subpass_list[name] = registerSubpass(subpass_attachments, external_attachments);
        }
        
        //---Render pass--- (only vulkan needs it)
        //:     r [s1 s2 s3]
        #if defined USE_VULKAN
        if (line.at(0) == "r") {
            if (line.size() != 2) log::error("The description of the renderpass is invalid, it has to be 'r [s1 s2]'");
            
            //: Subpasses
            std::vector<SubpassID> renderpass_subpasses{};
            std::vector<str> list = split(list_contents(line.at(1)));
            for (auto &sp : list) {
                if (not subpass_list.count(sp)) log::error("You have used an incorrect subpass name, %s", sp.c_str());
                renderpass_subpasses.push_back(subpass_list.at(sp));
            }
            
            //: Register renderpass
            registerRenderPass(renderpass_subpasses);
        }
        #endif
        
        //---Shaders---
        //:     d/p shader subpass vertices      d - draw shader, p - post shader
        if (line.at(0) == "d" or line.at(0) == "p") {
            if (line.size() != 4)
                log::error("The description of the shader is invalid, it has to be 'd/p shader subpass vertexdata'");
            
            //: Shader name
            ShaderID shader{line.at(1)};
            
            //: Shader type
            ShaderType type = line.at(0) == "d" ? SHADER_DRAW : SHADER_POST;
            
            //: Register shader
            Shader::registerShader(shader.value, type);
            
            //: Subpass
            str subpass_name = line.at(2);
            if (not subpass_list.count(subpass_name)) log::error("You have used an incorrect subpass name, %s", subpass_name.c_str());
            SubpassID subpass = subpass_list.at(subpass_name);
            Map::subpass_shader.add(subpass, shader);
            
            //: Register shader
            #if defined USE_VULKAN
                //: TODO: REFACTOR
                std::vector<std::pair<str, VertexInputRate>> vertex_descriptions = {{line.at(3), INPUT_RATE_VERTEX}};
                shaders.list.at(type).at(shader).pipeline = Shader::API::createGraphicsPipeline(shader, vertex_descriptions);
            #elif defined USE_OPENGL
                api.shaders.at(shader).subpass = subpass;
            #endif
        }
    }
}
