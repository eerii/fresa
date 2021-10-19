//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <tuple>


#define REGISTER_REFLECTION_INFO(Class) \
template<> \
struct ::Verse::Reflection<Class> \
{ \
    static constexpr bool has_reflection_info = true; \
    static constexpr char const* name{ #Class }; \
    static constexpr auto layout() { return ReflectionLayout<Class>::GetLayout(); } \
    static constexpr bool is_complex{ true }; \
};


#define StartReflection(Class) \
template<> \
struct ::Verse::ReflectionLayout<Class> \
{ \
    using Type = Class; \
    static constexpr auto GetLayout() \
    { \
        return std::make_tuple(

#define Member(Name) std::make_tuple(#Name, &Type::Name)

#define EndReflection(Class) \
        ); \
    } \
}; \
REGISTER_REFLECTION_INFO(Class)


namespace Verse
{

    //Reflection helpers
    template<typename T>
    struct ReflectionLayout {
        static constexpr auto GetLayout() {
            return std::make_tuple(
                std::make_tuple(
                    "#Unknown", "Unknown"
                )
            );
        }
    };

    template<typename T>
    struct Reflection {
        static constexpr bool has_reflection_info = false;
        static constexpr char const* name{ "Unknown" };
        static constexpr bool is_complex{ false };
        
        static constexpr auto layout() {
            return ReflectionLayout<T>::GetLayout();
        }
    };

    //Apply function to every element of a tuple
    template<
        typename Callable,
        typename Tuple,
        size_t... Indexes
    >
    constexpr void tuple_for_each(Tuple&& t, Callable&& f, std::index_sequence<Indexes...>)
    {
        (f(std::get<Indexes>(t)), ...);
    }

    template<
        typename Callable,
        typename... Args,
        template<typename...> typename Tuple,
        typename Is = std::make_index_sequence<sizeof...(Args)>
    >
    constexpr void tuple_for_each(Tuple<Args...>&& t, Callable&& f)
    {
        tuple_for_each(t, f, Is{});
    }
}
