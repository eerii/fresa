//* attachments
//      includes the definitions for attachments, subpasses and renderpasses
#pragma once

#include "r_api.h"
#include "fresa_math.h"
#include "fresa_enum.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: check for required interface definitions
    static_assert(requires { typename IImage; },                    "graphics api interface 'IImage' not defined");
    static_assert(requires { typename IImageView; },                "graphics api interface 'IImageView' not defined");
    static_assert(requires { typename IImageUsageFlags; },          "graphics api interface 'IImageUsageFlags' not defined");
    static_assert(requires { typename IImageAspectFlags; },         "graphics api interface 'IImageAspectFlags' not defined");
    static_assert(requires { typename IImageLayout; },              "graphics api interface 'IImageLayout' not defined");
    static_assert(requires { typename IFormat; },                   "graphics api interface 'IFormat' not defined");
    static_assert(requires { typename IAllocation; },               "graphics api interface 'IAllocation' not defined");
    static_assert(requires { typename IMemoryUsage; },              "graphics api interface 'IMemoryUsage' not defined");
    static_assert(requires { typename IAttachmentLoadOp; },         "graphics api interface 'IAttachmentLoadOp' not defined");
    static_assert(requires { typename IAttachmentStoreOp; },        "graphics api interface 'IAttachmentStoreOp' not defined");
    static_assert(requires { typename IAttachmentDescription; },    "graphics api interface 'IAttachmentDescription' not defined");

    static_assert(no_image_layout != 0, "graphics api interface 'no_image_layout' not defined");
    static_assert(no_format != 0,       "graphics api interface 'no_format' not defined");
    static_assert(no_usage != 0,        "graphics api interface 'no_usage' not defined");
    static_assert(no_aspect != 0,       "graphics api interface 'no_aspect' not defined");
    static_assert(no_memory_usage != 0, "graphics api interface 'no_memory_usage' not defined");

    //: attachment type
    enum struct AttachmentType {
        COLOR = 1 << 1,
        DEPTH = 1 << 2,
        INPUT = 1 << 3
    };
    
    // ·············
    // · DATATYPES ·
    // ·············

    //: texture data
    struct Texture {
        IImage image;
        IImageView image_view;

        IAllocation allocation;
        IMemoryUsage memory_usage = no_memory_usage;

        IFormat format = no_format;
        IImageUsageFlags usage = no_usage;
        IImageAspectFlags aspect = no_aspect;

        IImageLayout initial_layout = no_image_layout;
        IImageLayout final_layout = no_image_layout;
        
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

    //: subpass data
    struct Subpass {
        //- todo
    };

    //: renderpass data
    struct Renderpass {
        //- todo
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

    namespace subpass
    {
        //  implemented in r_api_*.cpp

        //: create subpass
        void create();
    }
}