//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_opengl.h"
#include "r_vulkan.h"
#include "events.h"
#include "bidirectional_map.h"
#include <set>

#define MAX_INDIRECT_COMMANDS 64

namespace Fresa::Graphics
{
#if defined USE_VULKAN
    using GraphicsAPI = Vulkan;
#elif defined USE_OPENGL
    using GraphicsAPI = OpenGL;
#endif
    
    inline WindowData win;
    inline CameraData camera;
    inline GraphicsAPI api;
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

    //---Drawing---
    TextureID registerTexture(const GraphicsAPI &api, Vec2<> size, Channels ch, ui8* pixels);
    
    inline std::map<GeometryBufferID, GeometryBufferData> geometry_buffer_data{};
    inline std::map<InstancedBufferID, InstancedBufferData> instanced_buffer_data{};
    inline std::map<TextureID, TextureData> texture_data{};
    inline std::map<DrawUniformID, DrawUniformData> draw_uniform_data{};
    
    inline DrawQueue draw_queue{};
    inline DrawIQueue draw_queue_instanced{};
    inline std::vector<DrawDescription*> draw_descriptions{};
    
    //---Indirect Drawing---
    IndirectBufferID registerIndirectCommandBuffer(const GraphicsAPI &api);
    void addIndirectDrawCommand(const GraphicsAPI &api, DrawDescription &description);
    void removeIndirectDrawCommand(const GraphicsAPI &api, DrawDescription &description);
    inline std::map<IndirectBufferID, IndirectCommandBuffer> draw_indirect_buffers{};
    
    //---Render passes and attachments---
    void processRendererDescription(GraphicsAPI &api, const WindowData &win);
    
    RenderPassID registerRenderPass(const GraphicsAPI &api, std::vector<SubpassID> subpasses);
    inline std::map<RenderPassID, RenderPassData> render_passes{};
    
    SubpassID registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list = {});
    inline std::map<SubpassID, SubpassData> subpasses{};
    
    AttachmentID registerAttachment(const GraphicsAPI &api, AttachmentType type, Vec2<> size);
    void recreateAttachments(const GraphicsAPI &api);
    inline std::map<AttachmentID, AttachmentData> attachments{};
    inline int render_attachment = -1; // -1: swapchain, -2: wireframe, 0...n: attachment
    bool hasMultisampling(AttachmentID attachment, bool check_samples = true);
    
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
    inline std::map<ShaderID, ShaderData> compute_shaders;
    inline ShaderID shadowmap_shader;
    void createShaderList();

    std::vector<char> readSPIRV(std::string filename);
    ShaderCode readSPIRV(const ShaderLocations &locations);
    ShaderData createShaderData(str name);
    ShaderCompiler getShaderCompiler(const std::vector<char> &code);
    
    void updateDrawDescriptorSets(const GraphicsAPI &api, const DrawDescription& draw);
    
    //---Other---
    void resize(GraphicsAPI &api, WindowData &win);

    void render(GraphicsAPI &api, WindowData &win, CameraData &cam);
    void present(GraphicsAPI &api, WindowData &win);

    void clean(GraphicsAPI &api);

    //---Attributes---
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VertexAttributeDescription> getAttributeDescriptions_(ui32 binding = 0, ui32 previous_location = 0) {
        std::vector<VertexAttributeDescription> attribute_descriptions{};
        
        log::graphics("");
        log::graphics("Creating attribute descriptions...");
        
        ui32 offset = 0;
        for_<Reflection::as_type_list<V>>([&](auto i){
            using T = std::variant_alternative_t<i.value, Reflection::as_type_list<V>>;
            str name = "";
            if constexpr (Reflection::is_reflectable<V>)
                name = str(V::member_names.at(i.value));
            
            int l = (int)attribute_descriptions.size();
            attribute_descriptions.resize(l + 1);
            
            attribute_descriptions[l].binding = binding;
            attribute_descriptions[l].location = l + previous_location;
            
            int size = sizeof(T);
            if (size % 4 != 0 or size < 4 or size > 16)
                log::error("Vertex data has an invalid size %d", size);
            attribute_descriptions[l].format = (VertexFormat)(size / 4);
            
            attribute_descriptions[l].offset = offset;
            
            log::graphics(" - Attribute %s [%d:%d] - Format : %d - Size : %d - Offset %d", name.c_str(), attribute_descriptions[l].binding,
                          attribute_descriptions[l].location, attribute_descriptions[l].format, size, offset);
            
            offset += size;
        });
        
        log::graphics("");
        return attribute_descriptions;
    }
    
    template <typename... V>
    std::vector<VertexAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attribute_descriptions{};
        ui32 binding = 0;
        ui32 previous_location = 0;
        
        ([&](){
            auto a = getAttributeDescriptions_<V>(binding++, previous_location);
            previous_location = (ui32)a.size();
            attribute_descriptions.insert(attribute_descriptions.end(), a.begin(), a.end());
        }(), ...);
        
        return attribute_descriptions;
    }
}
