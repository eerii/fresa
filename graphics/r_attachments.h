//* attachments
//      includes the definitions for attachments, subpasses and renderpasses
#pragma once

#include "r_api.h"
#include "fresa_math.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: interfaces for vulkan objects (allows for making this file mostly api agnostic in the future)
    using IImage = VkImage;
    using IImageView = VkImageView;
    using IImageUsageFlagBits = VkImageUsageFlagBits;
    using IImageAspectFlagBits = VkImageAspectFlagBits;
    using IImageLayout = VkImageLayout;
    using IFormat = VkFormat;
    using IImageLayout = VkImageLayout;
    using IAllocation = VmaAllocation;
    using IMemoryUsage = VmaMemoryUsage;
    using IAttachmentLoadOp = VkAttachmentLoadOp;
    using IAttachmentStoreOp = VkAttachmentStoreOp;
    using IAttachmentDescription = VkAttachmentDescription;

    //: attachment type
    enum struct AttachmentType {
        COLOR = 1 << 0,
        DEPTH = 1 << 1,
        INPUT = 1 << 2
    };

    // ·············
    // · DATATYPES ·
    // ·············

    //: texture data
    struct Texture {
        IImage image = VK_NULL_HANDLE;
        IImageView image_view = VK_NULL_HANDLE;

        IAllocation allocation = VK_NULL_HANDLE;
        IMemoryUsage memory_usage = VMA_MEMORY_USAGE_MAX_ENUM;

        IFormat format = VK_FORMAT_MAX_ENUM;
        IImageUsageFlagBits usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        IImageAspectFlagBits aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;

        IImageLayout initial_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
        IImageLayout final_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
        
        Vec2<ui32> size;
    };

    //: attachment data
    struct Attachment {
        AttachmentType type;
        Texture texture;

        IAttachmentLoadOp load_op;
        IAttachmentStoreOp store_op;
        IAttachmentDescription description;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace attachment
    {
        //  implemented in r_api_*.cpp

        //: construct a texture object from a set of parameters
        void buildTexture(Texture &texture);
    }
}