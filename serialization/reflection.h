//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

//thanks to Jameson Thatcher (bluescreenofdoom)[http://bluescreenofdoom.com/post/code/Reflection/],
//          veselink1 (refl-cpp)[https://github.com/veselink1/refl-cpp],
//          chochlik (static reflection)[https://isocpp.org/files/papers/n3996.pdf] and
//          KonanM (tser)[https://github.com/KonanM/tser]
//for the help with his part

#pragma once

#include <array>
#include <tuple>
#include <ostream>
#include <string_view>
#include <variant>

//---Reflection---
//      This macro is the closest thing to type reflection that we currently have in C++. It can be used like:
//      struct Test {
//          Serialize(Test, member);
//          int member;
//      };
//      It makes the struct reflectable, with a list of member_names, comparison operators and it allows the construction of special functions
//      like forEach() that iterates through all members, as well as implementations of serialization for saving/loading data and inspectors
//      It is also created at compile time, so it should not cost much at runtime

#define Serialize(Type, ...) \
\
inline decltype(auto) members() const { return std::tie(__VA_ARGS__); } \
inline decltype(auto) members() { return std::tie(__VA_ARGS__); } \
\
static constexpr std::array<char, ::Fresa::Reflection::str_size(#__VA_ARGS__)> member_name_data = [](){ \
    std::array<char, ::Fresa::Reflection::str_size(#__VA_ARGS__)> chars{'\0'}; \
    size_t _idx = 0; \
    constexpr auto* ini(#__VA_ARGS__); \
    for (char const* _c = ini; *_c; ++_c, ++_idx) \
        if(*_c != ',' && *_c != ' ') \
            chars[_idx] = *_c; \
    return chars;}(); \
\
static constexpr const char* type_name = #Type; \
\
static constexpr std::array<const char*, ::Fresa::Reflection::n_args(#__VA_ARGS__)> member_names = [](){ \
    std::array<const char*, ::Fresa::Reflection::n_args(#__VA_ARGS__)> out{ }; \
    for(size_t i = 0, n_args = 0; n_args < ::Fresa::Reflection::n_args(#__VA_ARGS__) ; ++i) { \
        while(Type::member_name_data[i] == '\0') i++; \
        out[n_args++] = &Type::member_name_data[i]; \
        while(Type::member_name_data[++i] != '\0'); } \
    return out;}(); \
\
static constexpr size_t size = ::Fresa::Reflection::n_args(#__VA_ARGS__); \
\
template<typename OT, std::enable_if_t<std::is_same_v<OT,Type> && !::Fresa::Reflection::is_detected<::Fresa::Reflection::t_equal, OT>, int> = 0> \
friend bool operator==(const Type& lhs, const OT& rhs) { return lhs.members() == rhs.members(); } \
template<typename OT, std::enable_if_t<std::is_same_v<OT,Type> && !::Fresa::Reflection::is_detected<::Fresa::Reflection::t_nequal, OT>, int> = 0> \
friend bool operator!=(const Type& lhs, const OT& rhs) { return !(lhs == rhs); } \
template<typename OT, std::enable_if_t<std::is_same_v<OT,Type> && !::Fresa::Reflection::is_detected<::Fresa::Reflection::t_smaller, OT>, int> = 0> \
friend bool operator< (const OT& lhs, const OT& rhs) { return ::Fresa::Reflection::less(lhs, rhs); } \
template<typename OT, std::enable_if_t<std::is_same_v<OT,Type> && !::Fresa::Reflection::is_detected<::Fresa::Reflection::t_print, OT>, int> = 0> \
friend std::ostream& operator<<(std::ostream& os, const OT& t) { ::Fresa::Reflection::printYAML<1>(os, t); return os; }

namespace Fresa
{
    namespace Reflection
    {
        //---C++20 is_detected implementation---
        struct ns {
            ~ns() = delete;
            ns(ns const&) = delete;
        };
    
        template <class Default, class AlwaysVoid, template<class...> class Op, class... Args>
        struct detector {
            using value_t = std::false_type;
            using type = Default;
        };
    
        template <class Default, template<class...> class Op, class... Args>
        struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
            using value_t = std::true_type;
            using type = Op<Args...>;
        };
    
        template<class T>
        struct is_array : std::is_array<T> {};
    
        template<template<typename, size_t> class TArray, typename T, size_t N>
        struct is_array<TArray<T, N>> : std::true_type {};
    
        constexpr size_t n_args(char const* c, size_t nargs = 1) {
            for (; *c; ++c) if (*c == ',') ++nargs;
            return nargs;
        }
    
        constexpr size_t str_size(char const* c, size_t str_size = 1) {
            for (; *c; ++c) ++str_size; return str_size;
        }
    
        template <template<class...> class Op, class... Args>
        constexpr bool is_detected = detector<ns, void, Op, Args...>::value_t::value;
    
        //---Properties---
        template<class T> using t_begin = decltype(*std::begin(std::declval<T>()));
        template<class T> constexpr bool is_container = is_detected<t_begin, T>;
    
        template<class T> using t_members = decltype(std::declval<T>().members());
        template<class T> constexpr bool is_reflectable = is_detected<t_members, T>;
        
        template<class T> using t_tuple = std::tuple_element_t<0, T>;
        template<class T> constexpr bool is_tuple = is_detected<t_tuple, T>;
    
        template<class T> using t_print = decltype(std::declval<std::ostream>() << std::declval<T>());
    
        template<class T> using t_optional = decltype(std::declval<T>().has_value());
        template<class T> using t_element = typename T::element_type;
        template<class T> using t_mapped = typename T::mapped_type;
        template<class T> constexpr bool is_pointer_like = std::is_pointer_v<T> || is_detected<t_element, T> || is_detected<t_optional, T>;
    
        template<class T> using t_smaller = decltype(std::declval<T>() < std::declval<T>());
        template<class T> using t_equal = decltype(std::declval<T>() == std::declval<T>());
        template<class T> using t_nequal = decltype(std::declval<T>() != std::declval<T>());
    
        template<class T> using t_component = decltype(std::declval<T>().component_name);
        
        template <class T, class U> struct is_in_variant;
        template <class T, class... Ts>
        struct is_in_variant<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };
    
        //---Tuple less operator---
        template<typename T>
        constexpr inline bool less(const T& lhs, const T& rhs);
    
        template< class T, std::size_t... I>
        constexpr inline bool less(const T& lhs, const T& rhs, std::index_sequence<I...>) {
            bool is_smaller = false;
            (void)((less(std::get<I>(lhs) , std::get<I>(rhs)) ? (static_cast<void>(is_smaller = true), false) :
                                                                (less(std::get<I>(rhs), std::get<I>(lhs)) ? false : true)) &&...);
            return is_smaller;
        }
    
        template<typename T>
        constexpr inline bool less(const T& lhs, const T& rhs) {
            if constexpr (is_reflectable<T>)
                return less(lhs.members(), rhs.members());
            if constexpr (is_tuple<T>)
                return less(lhs, rhs, std::make_index_sequence<std::tuple_size_v<T>>());
            if constexpr (is_container<T> && !is_detected<t_smaller, T>) {
                if (lhs.size() != rhs.size())
                    return lhs.size() < rhs.size();
                return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
            }
            if constexpr (std::is_enum_v<T>)
                return static_cast<std::underlying_type_t<T>>(lhs) < static_cast<std::underlying_type_t<T>>(rhs);
            return lhs < rhs;
        }
    
        //---For each---
        //      Recursively loop through each reflectable component, applying a callable function to it
        //      The parameter L indicates the level of the recursion, how far is the parent object
        //      The argument name serves to pass the name of the member to the next iteration, which can then be passed to the function
        template<std::size_t L = 0, typename Callable, typename T>
        constexpr void forEach(T &&t, Callable &&f, const char* name = "") {
            using V = std::decay_t<T>;
            
            if constexpr(is_reflectable<V>) {
                auto apply_to = [&](auto& ... m) {
                    std::size_t i = 0;
                    (forEach<L+1>(m, f, V::member_names[i++]), ...);
                };
                
                std::apply(apply_to, t.members());
            }
            
            f(t, L, name);
        }
        
        
        //---Some examples (need much more work)---
    
        //: Recursively print YAML
        template<std::size_t L>
        constexpr inline decltype(auto) getLevelSeparator(std::ostream& os) {
            for (std::size_t i = 0; i < 2*L; i++)
                os << ' ';
            return "";
        }
    
        template<std::size_t L, typename T>
        constexpr inline void printYAML(std::ostream& os, T&& val) {
            using V = std::decay_t<T>;
            
            if constexpr (std::is_constructible_v<std::string, T> || std::is_same_v<V, char>) {
                os << "\"" << val << "\"";
            } else if constexpr (is_container<V>) {
                size_t i = 0;
                os << "[";
                for (auto& elem : val)
                    os << (i++ == 0 ? "" : ",") << printYAML<L>(os, elem);
                os << "]";
            } else if constexpr (is_reflectable<V> && !is_detected<t_print, V>) {
                auto p_mem = [&](auto& ... member) {
                    size_t i = 0;
                    ((os << (i != 0 ? "\n" : ""),
                      getLevelSeparator<L>(os),
                      os << V::member_names[i++] << " : " << printYAML<L+1>(os, member)
                      ), ...);
                };
                if (L < 2) //Hide struct names for now, see what to do later
                    os << V::type_name << " : ";
                os << "\n";
                std::apply(p_mem, val.members());
            } else if constexpr (std::is_enum_v<V> && !is_detected<t_print, V>) {
                os << printYAML<L>(os, static_cast<std::underlying_type_t<V>>(val));
            } else if constexpr (is_tuple<V> && !is_detected<t_print, V>) {
                std::apply([&](auto& ... t) {
                    int i = 0; os << "{"; (((i++ != 0 ? os << ", " : os), printYAML<L>(os, t)), ...); os << "}";
                }, val);
            } else if constexpr (is_pointer_like<V>) {
                os << (val ? (os << (printYAML<L>(os, *val)), "") : "null");
            } else {
                os << val;
            }
        }
    
        //: Recursively print JSON
        template<typename T>
        constexpr inline decltype(auto) printJSON(std::ostream& os, T&& val) {
            using V = std::decay_t<T>;
            
            if constexpr (std::is_constructible_v<std::string, T> || std::is_same_v<V, char>) {
                os << "\"" << val << "\"";
            } else if constexpr (is_container<V>) {
                size_t i = 0;
                os << "[";
                for (auto& elem : val)
                    os << (i++ == 0 ? "" : ",") << ::Fresa::Reflection::printJSON(os, elem);
                os << "]";
            } else if constexpr (is_reflectable<V> && !is_detected<t_print, V>) {
                auto p_mem = [&](auto& ... member) {
                    size_t i = 0;
                    (((os << (i != 0 ? ", " : "") << '\"'), os << V::member_names[i++] << "\" : " << ::Fresa::Reflection::printJSON(os, member)), ...);
                };
                os << "{ \"" << V::type_name << "\": {"; std::apply(p_mem, val.members()); os << "}}";
            } else if constexpr (std::is_enum_v<V> && !is_detected<t_print, V>) {
                os << ::Fresa::Reflection::printJSON(os, static_cast<std::underlying_type_t<V>>(val));
            } else if constexpr (is_tuple<V> && !is_detected<t_print, V>) {
                std::apply([&](auto& ... t) {
                    int i = 0; os << "{"; (((i++ != 0 ? os << ", " : os), ::Fresa::Reflection::printJSON(os, t)), ...); os << "}";
                }, val);
            } else if constexpr (is_pointer_like<V>) {
                os << (val ? (os << (::Fresa::Reflection::printJSON(os, *val)), "") : "null");
            } else {
                os << val;
            }
            
            return "";
        }
    }
}
