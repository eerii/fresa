//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"

namespace Fresa::Graphics
{
    struct CommonAPI {
        //TODO: REFACTOR
        
        std::map<GeometryBufferID, GeometryBufferData> geometry_buffer_data;
        std::map<InstancedBufferID, InstancedBufferData> instanced_buffer_data;
        std::map<TextureID, TextureData> texture_data;
        std::map<DrawUniformID, DrawUniformData> draw_uniform_data;
        
        std::vector<DrawDescription*> draw_descriptions;
        
        std::map<IndirectBufferID, IndirectCommandBuffer> draw_indirect_buffers;
        
        std::map<RenderPassID, RenderPassData> render_passes;
        std::map<SubpassID, SubpassData> subpasses;
        std::map<AttachmentID, AttachmentData> attachments;
    };
}
