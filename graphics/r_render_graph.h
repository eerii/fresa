//* render graph
//      creates a render graph abstraction from a file
#pragma once

#include "r_api.h"
#include "r_attachments.h"

#include "strong_types.h"
#include "fresa_math.h"
#include "bidirectional_map.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: strong types for resource ids
    using RenderpassID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Hashable, strong::Formattable>;
    using AttachmentID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Hashable, strong::Formattable>;

    //: user defined literals for creating resource ids
    constexpr auto rid(str_view s) { return RenderpassID{ hash_fnv1a(s) }; }
    constexpr auto aid(str_view s) { return AttachmentID{ hash_fnv1a(s) }; }

    // ·············
    // · DATATYPES ·
    // ·············

    namespace rg
    {
        //: attachment description
        struct AttachmentDescription {
            AttachmentType type;
            Vec2<ui32> size;
            IFormat format;
            ui32 samples;
        };

        //: renderpass description
        struct RenderpassDescription {
            std::vector<AttachmentID> input_attachments;
            std::vector<AttachmentID> output_attachments;
            str_view shader;
        };
    }

    //: render graph
    struct RenderGraph {
        std::unordered_map<RenderpassID, rg::RenderpassDescription> renderpasses;
        std::unordered_map<AttachmentID, rg::AttachmentDescription> attachments;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace rg
    {
        RenderGraph loadRenderGraph();
    }
}