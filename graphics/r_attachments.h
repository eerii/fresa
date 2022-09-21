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
    using IImageUsageFlags = VkImageUsageFlags;
    using IImageAspectFlags = VkImageAspectFlags;
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
        COLOR = 1 << 1,
        DEPTH = 1 << 2,
        INPUT = 1 << 3
    };
    constexpr auto operator+(AttachmentType a) {return static_cast<std::underlying_type_t<AttachmentType>>(a);}
    constexpr AttachmentType operator| (AttachmentType a, AttachmentType b) { return AttachmentType(+a | +b); }
    constexpr std::underlying_type_t<AttachmentType> operator| (AttachmentType a, std::underlying_type_t<AttachmentType> b) { return +a | b; }
    constexpr std::underlying_type_t<AttachmentType> operator& (AttachmentType a, std::underlying_type_t<AttachmentType> b) { return +a & b; }

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
        IImageUsageFlags usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        IImageAspectFlags aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;

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

        //: create attachment
        Attachment create(AttachmentType type, Vec2<ui32> size);
    }
}