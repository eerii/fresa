//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_opengl.h"
#include "r_vulkan.h"

#include "r_shaders.h"

#include "events.h"
#include "bidirectional_map.h"

#include <set>

#define MAX_INDIRECT_COMMANDS 64

namespace Fresa::Graphics
{
    using GraphicsAPI = IF_VULKAN(Vulkan) IF_OPENGL(OpenGL);
    inline GraphicsAPI api;
}

//---API---
//      This coordinates the different rendering APIs. Here are defined the common functions that are later expanded in the respective source files
//      Right now it has full support for OpenGL and Vulkan.

namespace Fresa::Graphics
{
    namespace Common {
        BufferData allocateBuffer(ui32 size, BufferUsage usage, BufferMemory memory, void* data = nullptr, bool delete_with_program = true);
        void updateBuffer(BufferData &buffer, ui32 size, void* data);
        void copyBuffer(BufferData &src, BufferData &dst, ui32 size, ui32 offset = 0);
    }
    
    //: TODO: REFACTOR
    
    //---API---
    void configureAPI();
    void createAPI();

    //---Drawing---
    TextureID registerTexture(Vec2<ui16> size, Channels ch, ui8* pixels);
    
    //---Indirect Drawing---
    IndirectBufferID registerIndirectCommandBuffer();
    void addIndirectDrawCommand(DrawDescription &description);
    void removeIndirectDrawCommand(DrawDescription &description);
    
    //---Render passes and attachments---
    void processRendererDescription();
    
    RenderPassID registerRenderPass(std::vector<SubpassID> subpasses);
    
    SubpassID registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list = {});
    
    AttachmentID registerAttachment(AttachmentType type, Vec2<ui16> size);
    void recreateAttachments();
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
    void updateDrawDescriptorSets(const DrawDescription& draw);
    
    //---Other---
    void resize();

    void render();
    void present();

    void clean();

    //---Attributes---
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VertexAttributeDescription> getAttributeDescriptions(ui32 binding = 0, ui32 previous_location = 0) {
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
}
