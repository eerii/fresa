//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_bufferdata.h"
#include <glm/glm.hpp>
#include <vector>
#include <optional>

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#endif

namespace Verse::Graphics
{
    using DrawID = ui32;
    using DrawBufferID = ui32;
    using TextureID = ui32;

    enum Channels {
        TEXTURE_CHANNELS_G = 1,
        TEXTURE_CHANNELS_GA = 2,
        TEXTURE_CHANNELS_RGB = 3,
        TEXTURE_CHANNELS_RGBA = 4
    };

    struct TextureData {
        int w, h, ch;
    #if defined USE_OPENGL
        ui32 id_;
    #elif defined USE_VULKAN
        VkImage image;
        VmaAllocation allocation;
        VkFormat format;
        VkImageLayout layout;
        VkImageView image_view;
    #endif
    };

    struct DrawBufferData {
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_size;
    };

    struct DrawData {
        DrawBufferID buffer_id;
        std::vector<BufferData> uniform_buffers;
        std::optional<TextureID> texture_id;
        #ifdef USE_VULKAN
        std::vector<VkDescriptorSet> descriptor_sets;
        #endif
    };

    struct DrawQueueInfo {
        const DrawBufferData* buffer;
        const DrawData* data;
        glm::mat4 model;
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}
