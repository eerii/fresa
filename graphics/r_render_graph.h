//* render_graph
//      includes the attachment, subpass and renderpass abstraction and how they are connected
#pragma once

#include "strong_types.h"
#include "fresa_math.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: strong types for resource ids
    using RenderpassID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Hashable, strong::Formattable>;
    using AttachmentID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Hashable, strong::Formattable>;

    //: user defined literals for creating resource ids
    consteval auto operator""_rid (const char* str, size_t len) { return RenderpassID{ hash_fnv1a(str_view{str, len}) }; }
    consteval auto operator""_aid (const char* str, size_t len) { return AttachmentID{ hash_fnv1a(str_view{str, len}) }; }

    //: rendergraph concept
    namespace concepts
    {
        template <typename T>
        concept RenderGraphList = requires (T t) {
            T::count;
            T::list;
        }; //! specialization of

        template <typename T>
        concept RenderGraph = requires (T t) {
            typename T::RenderpassList;
            typename T::AttachmentList;
        };
    }

    // ·············
    // · DATATYPES ·
    // ·············

    //: render graph list
    //      a consteval container that can hold a list of attachments, renderpasses...
    template <auto id, typename Previous>
    requires (not concepts::RenderGraphList<Previous> or std::is_same_v<decltype(id), typename Previous::type>)
    struct RenderGraphList {
        //: count
        constexpr static ui32 count = []{
            if constexpr (concepts::RenderGraphList<Previous>)
                return Previous::count + 1;
            else
                return 0;
        }();

        //: type (if it is the first item, the type is Previous, otherwise it is the type of the previous item)
        using type = decltype([]{
            if constexpr (concepts::RenderGraphList<Previous>)
                return (typename Previous::type){};
            else
                return Previous{};
        }());

        //: recursive list
        constexpr static std::array<type, count> list = []{
            if constexpr (count == 0)
                return std::array<type, 0>{};
            else {
                std::array<type, count> l;
                for (ui32 i = 0; i < count-1; ++i)
                    l.at(i) = Previous::list.at(i);
                l.at(count-1) = id;
                return l;
            }
        }();
    };

    //: render graph
    //      contains lists with all attachments, subpasses and renderpasses
    template <concepts::RenderGraphList R = RenderGraphList<""_rid, RenderpassID>,
              concepts::RenderGraphList A = RenderGraphList<""_aid, AttachmentID>>
    struct RenderGraph {
        using RenderpassList = R;
        using AttachmentList = A;

        consteval static auto& renderpasses() { return R::list; }
        consteval static auto& attachments() { return A::list; }
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace detail
    {
        //: add a renderpass to the rendergraph
        template <RenderpassID rid, concepts::RenderGraph Graph>
        consteval auto addPassImpl(Graph g) {
            //: check if renderpass is already in the list
            for_<0, Graph::RenderpassList::count>([&](auto i) {
                static_assert(Graph::renderpasses().at(i) != rid, "renderpass already in rendergraph");
            });

            return RenderGraph<RenderGraphList<rid, typename Graph::RenderpassList>, typename Graph::AttachmentList>{};
        }

        //: add an attachment to the rendergraph
        template <AttachmentID aid, concepts::RenderGraph Graph>
        consteval auto addAttachmentImpl(Graph g) {
            //: check if attachment is already in the list
            for_<0, Graph::AttachmentList::count>([&](auto i) {
                static_assert(Graph::attachments().at(i) != aid, "attachment already in rendergraph");
            });

            return RenderGraph<typename Graph::RenderpassList, RenderGraphList<aid, typename Graph::AttachmentList>>{};
        }
    }

    //: renderpass and attachment constexpr objects for the pipe operator
    template <RenderpassID rid> struct addPass { constexpr static RenderpassID id = rid; };
    template <AttachmentID aid> struct addAttachment { constexpr static AttachmentID id = aid; };

    namespace concepts
    {
        template <typename T> concept AddPass = ::fresa::concepts::SpecializationOfV<T, addPass>;
        template <typename T> concept AddAttachment = ::fresa::concepts::SpecializationOfV<T, addAttachment>;
    }

    //: pipe operators for adding resources to render graph
    template <concepts::RenderGraph Graph, concepts::AddPass R>
    consteval auto operator|(Graph g, R r) { return detail::addPassImpl<R::id>(g); }

    template <concepts::RenderGraph Graph, concepts::AddAttachment A>
    consteval auto operator|(Graph g, A a) { return detail::addAttachmentImpl<A::id>(g); }
}