//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_opengl.h"
#include "r_vulkan.h"
#include "events.h"
#include "bimap.h"
#include <set>

namespace Fresa::Graphics
{
#if defined USE_VULKAN
    using GraphicsAPI = Vulkan;
#elif defined USE_OPENGL
    using GraphicsAPI = OpenGL;
#endif
}

//---API---
//      This coordinates the different rendering APIs. Here are defined the common functions that are later expanded in the respective source files
//      Right now it has full support for OpenGL and Vulkan.

namespace Fresa::Graphics::API
{
    WindowData createWindow(Vec2<ui32> size, str name);
    ui16 getRefreshRate(WindowData &win, bool force = false);
    
    void configureAPI();
    GraphicsAPI createAPI(WindowData &win);

    //---Textures---
    TextureID registerTexture(const GraphicsAPI &api, Vec2<> size, Channels ch, ui8* pixels);
    DrawID registerDrawData(GraphicsAPI &api, DrawBufferID buffer, Shaders shader);
    inline std::map<DrawBufferID, DrawBufferData> draw_buffer_data{};
    inline std::map<TextureID, TextureData> texture_data{};
    inline std::map<DrawID, DrawData> draw_data{};
    inline DrawQueueMap draw_queue{};
    
    //---Render passes and attachments---
    AttachmentID registerAttachment(const GraphicsAPI &api, AttachmentType type, Vec2<> size);
    void recreateAttachments(const GraphicsAPI &api);
    inline std::map<AttachmentID, AttachmentData> attachments{};
    
    SubpassID registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list = {});
    inline std::map<SubpassID, SubpassData> subpasses{};
    
    RenderPassID registerRenderPass(Vulkan &vk, std::vector<SubpassID> subpasses);
    inline std::map<RenderPassID, RenderPassData> render_passes{};

    //---Shaders---
    void updateDescriptorSets(const GraphicsAPI &api, const DrawData* draw);

    std::vector<char> readSPIRV(std::string filename);
    ShaderCode readSPIRV(const ShaderLocations &locations);
    ShaderData createShaderData(str name);
    ShaderCompiler getShaderCompiler(const std::vector<char> &code);

    //---Other---
    void resize(GraphicsAPI &api, WindowData &win);

    void render(GraphicsAPI &api, WindowData &win, CameraData &cam);
    void present(GraphicsAPI &api, WindowData &win);

    void clean(GraphicsAPI &api);

    //---Templates---
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VertexAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attribute_descriptions = {};
        
        log::graphics("");
        log::graphics("Creating attribute descriptions...");
        
        ui32 offset = 0;
        Reflection::forEach(V{}, [&](auto &&x, ui8 level, const char* name){
            if (level == 1) {
                int i = (int)attribute_descriptions.size();
                attribute_descriptions.resize(i + 1);
                
                attribute_descriptions[i].binding = 0;
                attribute_descriptions[i].location = i;
                
                int size = sizeof(x);
                if (size % 4 != 0 or size < 4 or size > 16)
                    log::error("Vertex data has an invalid size %d", size);
                attribute_descriptions[i].format = (VertexFormat)(size / 4);
                
                attribute_descriptions[i].offset = offset;
                
                log::graphics(" - Attribute %s [%d] - Format : %d - Size : %d - Offset %d", name, i, attribute_descriptions[i].format, size, offset);
                
                offset += sizeof(x);
            }
        });
        
        log::graphics("");
        return attribute_descriptions;
    }
    
    //---Mappings---
    namespace Mappings {
        inline bi_map_AvB_BA<RenderPassID, SubpassID> renderpass_subpass;
        inline bi_map_AvB_BvA<SubpassID, AttachmentID> subpass_attachment;
        inline bi_map_AvB_BvA<SubpassID, Shaders> subpass_shader;
        
        struct renderpass {
            RenderPassID r_id;
            renderpass(RenderPassID i) : r_id(i) {}
            
            auto get_subpasses() {
                return getBimapAtoB_v(r_id, renderpass_subpass, subpasses);
            }
            auto get_attachments() {
                auto subpass_list = getBimapAtoB<SubpassID>(r_id, renderpass_subpass);
                std::map<AttachmentID, const AttachmentData&> attachment_list;
                for (auto s_id : subpass_list)
                    attachment_list.merge(getBimapAtoB_v(s_id, subpass_attachment, attachments));
                return attachment_list;
            }
            auto get_shaders() {
                auto subpass_list = getBimapAtoB<SubpassID>(r_id, renderpass_subpass);
                std::set<Shaders> shader_list;
                for (auto s_id : subpass_list) {
                    auto list = getBimapAtoB<Shaders>(s_id, subpass_shader);
                    shader_list.insert(list.begin(), list.end());
                }
                return std::vector<Shaders>{shader_list.begin(), shader_list.end()};
            }
        };
        
        struct subpass {
            SubpassID s_id;
            subpass(SubpassID i) : s_id(i) {}
            
            auto get_renderpasses() {
                return getBimapBtoA_v(s_id, renderpass_subpass, render_passes);
            }
            auto get_attachments() {
                return getBimapAtoB_v(s_id, subpass_attachment, attachments);
            }
            auto get_shaders() {
                return getBimapAtoB<Shaders>(s_id, subpass_shader);
            }
        };
        
        struct attachment {
            AttachmentID a_id;
            attachment(AttachmentID i) : a_id(i) {}
            
            auto get_subpasses() {
                return getBimapBtoA_v(a_id, subpass_attachment, subpasses);
            }
            auto get_renderpasses() {
                auto subpass_list = getBimapBtoA<SubpassID>(a_id, subpass_attachment);
                std::map<RenderPassID, const RenderPassData&> renderpass_list;
                for (auto s_id : subpass_list)
                    renderpass_list.merge(getBimapBtoA_v(s_id, renderpass_subpass, render_passes));
                return renderpass_list;
            }
            auto get_shaders() {
                auto subpass_list = getBimapBtoA<SubpassID>(a_id, subpass_attachment);
                std::set<Shaders> shader_list;
                for (auto s_id : subpass_list) {
                    auto list = getBimapAtoB<Shaders>(s_id, subpass_shader);
                    shader_list.insert(list.begin(), list.end());
                }
                return std::vector<Shaders>{shader_list.begin(), shader_list.end()};
            }
        };
        
        struct shader {
            Shaders sh;
            shader(Shaders i) : sh(i) {}
            
            auto get_subpasses() {
                return getBimapBtoA_v(sh, subpass_shader, subpasses);
            }
            auto get_renderpasses() {
                auto subpass_list = getBimapBtoA<SubpassID>(sh, subpass_shader);
                std::map<RenderPassID, const RenderPassData&> renderpass_list;
                for (auto s_id : subpass_list)
                    renderpass_list.merge(getBimapBtoA_v(s_id, renderpass_subpass, render_passes));
                return renderpass_list;
            }
            auto get_attachments() {
                auto subpass_list = getBimapBtoA<SubpassID>(sh, subpass_shader);
                std::map<AttachmentID, const AttachmentData&> attachment_list;
                for (auto s_id : subpass_list)
                    attachment_list.merge(getBimapAtoB_v(s_id, subpass_attachment, attachments));
                return attachment_list;
            }
        };
    };
}
