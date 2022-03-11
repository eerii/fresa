//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_opengl.h"
#include "r_vulkan.h"
#include "events.h"
#include "bidirectional_map.h"
#include <set>

namespace Fresa::Graphics
{
#if defined USE_VULKAN
    using GraphicsAPI = Vulkan;
#elif defined USE_OPENGL
    using GraphicsAPI = OpenGL;
#endif
    
    //TODO: Refactor the graphics API, use a BaseAPI and derived Vulkan and OpenGL classes.
    //      Also in API functions do not pass the &api, create an inline global member, research multithread
    /*struct BaseAPI {
        
    };*/
}

//---API---
//      This coordinates the different rendering APIs. Here are defined the common functions that are later expanded in the respective source files
//      Right now it has full support for OpenGL and Vulkan.

namespace Fresa::Graphics::API
{
    WindowData createWindow(Vec2<ui32> size, str name);
    ui16 getRefreshRate(WindowData &win, bool force = false);
    float getDPI();
    UniformBufferObject getScaledWindowUBO(const WindowData &win);
    
    void configureAPI();
    GraphicsAPI createAPI(WindowData &win);

    //---Textures---
    TextureID registerTexture(const GraphicsAPI &api, Vec2<> size, Channels ch, ui8* pixels);
    DrawID registerDrawData(GraphicsAPI &api, DrawBufferID buffer, ShaderID shader);
    inline std::map<DrawBufferID, DrawBufferData> draw_buffer_data{};
    inline std::map<TextureID, TextureData> texture_data{};
    inline std::map<DrawID, DrawData> draw_data{};
    inline DrawQueueMap draw_queue{};
    
    //---Render passes and attachments---
    void processRendererDescription(GraphicsAPI &api, const WindowData &win);
    inline str renderer_description_path = "";
    
    RenderPassID registerRenderPass(const GraphicsAPI &api, std::vector<SubpassID> subpasses);
    inline std::map<RenderPassID, RenderPassData> render_passes{};
    
    SubpassID registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list = {});
    inline std::map<SubpassID, SubpassData> subpasses{};
    
    AttachmentID registerAttachment(const GraphicsAPI &api, AttachmentType type, Vec2<> size);
    void recreateAttachments(const GraphicsAPI &api);
    inline std::map<AttachmentID, AttachmentData> attachments{};
    
    //---Mappings---
    namespace Map {
        inline map_AvB_BA<RenderPassID, SubpassID> renderpass_subpass;
        inline map_AvB_BvA<SubpassID, AttachmentID> subpass_attachment;
        inline map_AvB_BA<SubpassID, ShaderID> subpass_shader;
        
        inline auto renderpass_attachment = map_chain(renderpass_subpass, subpass_attachment);
        inline auto renderpass_shader = map_chain(renderpass_subpass, subpass_shader);
    };

    //---Shaders---
    inline std::map<ShaderID, ShaderData> shaders;
    void createShaderList();

    std::vector<char> readSPIRV(std::string filename);
    ShaderCode readSPIRV(const ShaderLocations &locations);
    ShaderData createShaderData(str name);
    ShaderCompiler getShaderCompiler(const std::vector<char> &code);
    
    void updateDescriptorSets(const GraphicsAPI &api, const DrawData* draw);

    //---Other---
    void resize(GraphicsAPI &api, WindowData &win);

    void render(GraphicsAPI &api, WindowData &win, CameraData &cam);
    void present(GraphicsAPI &api, WindowData &win);

    void clean(GraphicsAPI &api);

    //---Templates---
    template<typename T>
    void updateUniformBuffer(GraphicsAPI &api, BufferData buffer, const T& ubo) {
        #if defined USE_VULKAN
        void* data;
        vmaMapMemory(api.allocator, buffer.allocation, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vmaUnmapMemory(api.allocator, buffer.allocation);
        #elif defined USE_OPENGL
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        #endif
    }
    
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VertexAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attribute_descriptions = {};
        
        log::graphics("");
        log::graphics("Creating attribute descriptions...");
        
        ui32 offset = 0;
        for_<Reflection::as_type_list<V>>([&](auto i){
            using T = std::variant_alternative_t<i.value, Reflection::as_type_list<V>>;
            str name = "";
            if constexpr (Reflection::is_reflectable<V>)
                name = str(V::member_names.at(i.value));
            
            int j = (int)attribute_descriptions.size();
            attribute_descriptions.resize(j + 1);
            
            attribute_descriptions[j].binding = 0;
            attribute_descriptions[j].location = j;
            
            int size = sizeof(T);
            if (size % 4 != 0 or size < 4 or size > 16)
                log::error("Vertex data has an invalid size %d", size);
            attribute_descriptions[j].format = (VertexFormat)(size / 4);
            
            attribute_descriptions[j].offset = offset;
            
            log::graphics(" - Attribute %s [%d] - Format : %d - Size : %d - Offset %d", name.c_str(), j, attribute_descriptions[j].format, size, offset);
            
            offset += size;
        });
        
        log::graphics("");
        return attribute_descriptions;
    }
}
