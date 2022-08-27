//* reflection
//      implementing reflection into fresa has been one of the most iterative and extensive tasks in the engine
//      reflection in c++ as of now is pretty much absent, and there have been a multitude of hacks to get it kind of working
//      in fresa, i want reflection to be pretty much invisible to the user, so they don't have to write any extra code
//      but also to leverage the power of compile time reflection. these two requirements combined make it very difficult to implement
//
//      the brief history of how reflection was implemented begins with a "simple" macro system that pasted a lot of static code
//      inside every struct that the user wanted to be reflected. the user needed to input the name of all fields and then that information
//      could be queried. this was later moved to a template inheritance relation with leverage of the latest language features to avoid
//      macros alltogether, but the user still needed to input the name of all fields twice. then i implemented alexandr poltavsky's type loophole,
//      that allowed for reflection on everything except for member field names without any user intervention. the implementation now has moved
//      to something similar to antony polukhin's pfr library, but without as many features and updated to use concepts. reflection is ongoing
//      development, and the challenge of generating field names is still there. i might look into a compiler tool to optionally generate them
//      automatically during builds, since they are only required for debug interfaces such as the inspector.
//
//      this journey would have not been possible if not for the incredible contributions of different authors:
//      + alexandr poltavsky: [type loophole](https://github.com/alexpolt/luple/blob/master/type-loophole.h)
//      + antony polukhin: [pfr](https://github.com/apolukhin/pfr_non_boost)
//      + konanm [tser](https://github.com/KonanM/tser)
//      + jameson thatcher [bluescreenofdoom](http://bluescreenofdoom.com/post/code/Reflection)
//      + veselink1 [refl-cpp](https://github.com/veselink1/refl-cpp)
//      + fabian jung [tsmp](https://github.com/fabian-jung/tsmp)
//      + joao baptista [counting fields](https://towardsdev.com/counting-the-number-of-fields-in-an-aggregate-in-c-20-c81aecfd725c)
#pragma once

#include "std_types.h"
#include "string_utils.h"
#include "struct_fields.h"
#include "tie_as_tuple.h"
#include "constexpr_for.h"

namespace fresa
{
    //* get member
    template <std::size_t I, concepts::Aggregate T>
    [[nodiscard]] constexpr auto get(T& value) noexcept {
        return std::get<I>(tie_as_tuple(value));
    }
    template <std::size_t I, concepts::Aggregate T>
    [[nodiscard]] constexpr auto get(const T& value) noexcept {
        return std::get<I>(tie_as_tuple(value));
    }

    //* for each element (extends tuple like constexpr for)
    template <typename F, typename T> requires (not concepts::TupleLike<std::remove_reference_t<T>> and concepts::Aggregate<std::remove_reference_t<T>>)
    constexpr void for_(F&& f, T&& value) {
        for_(std::forward<F>(f), std::forward<decltype(tie_as_tuple(value))>(tie_as_tuple(value)));
    }

    //* concepts
    namespace concepts
    {
        //: equality
        template<typename T>
        concept EqualityComparable = requires(T value) { value == value; };

        //: hashable
        template<typename T>
        concept Hashable = requires(T value) {
            { std::hash<T>{}(value) } -> std::convertible_to<std::size_t>;
        };
    }

    //* equality operators
    template <concepts::Aggregate T> [[nodiscard]] constexpr bool operator==(const T& lhs, const T& rhs) { 
        if constexpr (concepts::EqualityComparable<T>)
            return lhs == rhs;
        else {
            bool result = true;
            constexpr auto fields = field_count_v<T>;
            for_<0, fields>([&](auto i) {
                result &= get<i>(lhs) == get<i>(rhs);
            });
            return result;
        }
    }
    template <concepts::Aggregate T> [[nodiscard]] constexpr bool operator!=(const T& lhs, const T& rhs) { return not (lhs == rhs); }

    //* member names
    //      meant to be overloaded by functions automatically generated using tools/reflection_names.py
    template <concepts::Aggregate T> constexpr auto field_names() {
        return std::array<str_view, 0>{};
    }

    namespace detail
    {
        //: get member name at index
        template <std::size_t I, concepts::Aggregate T> [[nodiscard]] constexpr str_view getNameWithIndex() {
            static_assert(I < field_count_v<T>, "index out of bounds");
            if constexpr (field_names<T>().size() > 0)
                return field_names<T>()[I];
            else
                return "";
        }
        
        //: get index of member name
        template <str_literal M, concepts::Aggregate T> [[nodiscard]] constexpr auto getIndexWithName() {
            constexpr auto fields = field_count_v<T>;
            std::optional<std::size_t> index;
            for_<0, fields>([&](auto i) {
                if (getNameWithIndex<i, T>() == M.value)
                    index = i;
            });
            return index;
        }
    }

    //* get member by name
    template <str_literal M, concepts::Aggregate T>
    [[nodiscard]] constexpr auto get(T& value) {
        constexpr auto index = detail::getIndexWithName<M, T>();
        static_assert(index.has_value(), "trying to access an invalid member");
        return get<index.value()>(value);
    }
}

//: std hashable type
namespace std
{
    template <fresa::concepts::Aggregate T> requires (not fresa::concepts::Hashable<T>)
    struct hash<T> {
        constexpr std::size_t operator()(const T& value) const {
            std::size_t result = 0;
            constexpr auto fields = fresa::field_count_v<T>;
            fresa::for_<0, fields>([&](auto i) {
                using Field = std::remove_reference_t<decltype(fresa::get<i>(value))>;
                static_assert(fresa::concepts::Hashable<Field>, "all the fields in the struct must be hashable");
                result ^= std::hash<Field>{}(fresa::get<i>(value));
            });
            return result;
        }
    };
}

//: fmt formatable type
#if __has_include("fmt/format.h")
    #include "fmt/format.h"

    namespace fresa::detail
    {
        template <fresa::concepts::Aggregate T>
        struct format_refl_impl {
            template<typename ParseContext>
            constexpr auto parse(ParseContext& ctx) {
                return ctx.begin();
            }

            template <typename FormatContext>
            constexpr auto format(const T& c, FormatContext& ctx) noexcept {
                fmt::format_to(ctx.out(), "{{");
                for_<0, field_count_v<T>>([&](auto i) {
                    fmt::format_to(ctx.out(), i > 0 ? ", {}" : "{}", get<i>(c));
                });
                return fmt::format_to(ctx.out(), "}}");
            }
        };

        template <fresa::concepts::Aggregate T>
        struct format_refl_names_impl {
            template<typename ParseContext>
            constexpr auto parse(ParseContext& ctx) {
                return ctx.begin();
            }

            template <typename FormatContext>
            constexpr auto format(const T& c, FormatContext& ctx) noexcept {
                fmt::format_to(ctx.out(), "{{");
                for_<0, field_count_v<T>>([&](auto i) {
                    fmt::format_to(ctx.out(), "{}{}: {}", i > 0 ? ", " : "", detail::getNameWithIndex<i, T>(), get<i>(c));
                });
                return fmt::format_to(ctx.out(), "}}");
            }
        };
    }

    template <fresa::concepts::Aggregate T> requires (std::is_class_v<T> and fresa::field_names<T>().size() == 0)
    struct fmt::formatter<T> : fresa::detail::format_refl_impl<T> {};

    template <fresa::concepts::Aggregate T> requires (std::is_class_v<T> and fresa::field_names<T>().size() > 0)
    struct fmt::formatter<T> : fresa::detail::format_refl_names_impl<T> {};
#endif