//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_bufferdata.h"

namespace Verse::Graphics
{
    using DrawID = ui32;

    struct DrawData {
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_size;
        std::vector<BufferData> uniform_buffers;
        std::vector<VkDescriptorSet> descriptor_sets;
    };
}
