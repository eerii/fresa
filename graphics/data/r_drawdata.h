//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_bufferdata.h"
#include <glm/glm.hpp>

namespace Verse::Graphics
{
    using DrawID = ui32;
    using DrawBufferID = ui32;

    struct DrawBuffer {
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_size;
    };

    struct DrawData {
        DrawBufferID buffer_id;
        std::vector<BufferData> uniform_buffers;
        std::vector<VkDescriptorSet> descriptor_sets;
    };

    struct DrawQueueInfo {
        const DrawBuffer* buffer;
        const DrawData* data;
        glm::mat4 model;
    };
}
