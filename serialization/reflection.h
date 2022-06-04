//: fresa by jose pazos perez, licensed under GPLv3
//  Reflection in C++ is (as of now) pretty much absent, however, I wanted to implemented it to use with automatic serialization and other things.
//  It took a lot of trial and error and many failed attempts, and it definitely has more room to grow, but I'm happy with the current result.
//  I read many articles and source code while working on this, and I'd like to list the most useful ones here. Thank you so much!
//      - Jameson Thatcher (bluescreenofdoom)[http://bluescreenofdoom.com/post/code/Reflection/]
//      - KonanM (tser)[https://github.com/KonanM/tser]
//      - veselink1 (refl-cpp)[https://github.com/veselink1/refl-cpp]
//      - chochlik (static reflection)[https://isocpp.org/files/papers/n3996.pdf]
//      - Alexandr Poltavsky (type loophole)[https://github.com/alexpolt/luple/blob/master/type-loophole.h]
//      - Fabian Jung (tsmp)https://github.com/fabian-jung/tsmp
//  In the future I'd like to keep improving this part of the project and maybe add a compiler tool to generate the reflection data.

#pragma once

#ifndef USING_REFLECTION
#define USING_REFLECTION

#include <array>
#include <tuple>
#include <ostream>
#include <string_view>
#include <numeric>
#include "string_helper.h"

//---Reflection---
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
    
        template <template<class...> class Op, class... Args>
        constexpr bool is_detected = detector<ns, void, Op, Args...>::value_t::value;
    
        //---Properties---
        template<class T> using t_begin = decltype(*std::begin(std::declval<T>()));
        template<class T> constexpr bool is_container = is_detected<t_begin, T>;
    
        template<class T> using t_members = decltype(std::declval<T>().member_names);
        template<class T> constexpr bool is_reflectable = is_detected<t_members, T>;
        
        template<class T> using t_tuple = std::tuple_element_t<0, T>;
        template<class T> constexpr bool is_tuple = is_detected<t_tuple, T>;
    
        template<class T> using t_optional = decltype(std::declval<T>().has_value());
        template<class T> using t_element = typename T::element_type;
        template<class T> using t_mapped = typename T::mapped_type;
        template<class T> constexpr bool is_pointer_like = std::is_pointer_v<T> || is_detected<t_element, T> || is_detected<t_optional, T>;
        
        //---Helpers---
        constexpr size_t n_args(char const* c, size_t nargs = 1) {
            for (; *c; ++c) if (*c == ',') ++nargs;
            return nargs;
        }
    
        constexpr size_t str_size(char const* c, size_t str_size = 1) {
            for (; *c; ++c) ++str_size; return str_size;
        }
   
        //---Member names---
        //      struct Test : Reflection::Members<"member"> {
        //          int member;
        //      };
        //: See a macro alternative that doesn't use inheritance below
        template<Str... M>
        struct Members {
            static constexpr std::array<const char*, sizeof...(M)> member_names { M.c_str()... };
        };
        
        //: Exclude certain functions from the type reflection
        //:     - Member names struct dependency
        template<class M> struct is_excluded {
            static constexpr bool value = type_name_n<M>().find("Reflection::Members"sv) != std::string_view::npos;
        };
        
        //---Struct type list---
        //: Generate friend declarations, t returns the type with auto and ct helps avoiding multiple definitions
        template<typename T, int N>
        struct tag {
            friend auto t(tag<T,N>);
            constexpr friend int ct(tag<T,N>);
        };
        
        //: Define friend functions, t now returns the type U, while ct returns 0 to avoid the multiple instantiations
        template<typename T, typename U, int N, bool B>
        struct tag_def {
            friend auto t(tag<T,N>) {
                return U{};
            }
            constexpr friend int ct(tag<T,N>) { return 0; }
        };
        template<typename T, typename U, int N>
        struct tag_def<T, U, N, true> {};
        
        //: Conversion operator, triggers the instantiation
        //: U is a parameter used to avoid cached template arguments, which does the detection using the friend functions and SFINAE
        //: Using sizeof with int and char seems to be a more reliable way of type checking for SFINAE
        template<typename T, int N>
        struct conversion {
            template<typename U, int M>
            static auto instantiate(...) -> int;
            
            template<typename U, int M, int = ct(tag<T,M>{})>
            static auto instantiate(int) -> char;
            
            template<typename U, int = sizeof(tag_def<T, U, N, sizeof(instantiate<U, N>(0)) == sizeof(char)>)>
            operator U();
        };
        
        //: Detects the data type field number, only works with aggregate types (no constructor, no virtual types, no private members)
        template<typename T, int... NN, std::enable_if_t<std::is_aggregate_v<T>, bool> = true>
        constexpr int detect(...) {
            return sizeof...(NN)-1;
        }
        template<typename T, int... NN>
        constexpr auto detect(int) -> decltype(T{ conversion<T,NN>{}... }, 0) {
            return detect<T, NN..., sizeof...(NN)>(0);
        }
        
        //: Get a variant type list from the type
        template<typename T, typename U>
        struct type_list;
        template<typename T, int... I>
        struct type_list< T, std::integer_sequence<int, I...> > {
            using type = typename filter_<is_excluded, decltype(t(tag<T, I>{}))...>::value;
        };
        template<typename T>
        using as_type_list = typename type_list<T, std::make_integer_sequence<int, detect<T>(0)>>::type;
        
        //: Size of each member
        template<typename U>
        struct size_list;
        template<typename... Ts>
        struct size_list< std::variant<Ts...> > {
            static constexpr std::array<size_t, sizeof...(Ts)> size{ sizeof(Ts)... };
        };
        template<typename T>
        constexpr auto as_size_list = size_list<as_type_list<T>>::size;
        
        //: Offsets for each member
        template<typename T, size_t index>
        constexpr size_t get_offset_c() {
            constexpr size_t s = std::accumulate(as_size_list<T>.begin(), as_size_list<T>.begin() + index, 0);
            return s;
        }
        
        template<typename T>
        size_t get_offset(size_t index) {
            size_t s = std::accumulate(as_size_list<T>.begin(), as_size_list<T>.begin() + index, 0);
            return s;
        }
        
        //: Get member index by name (constexpr with string literal or runtime with string)
        template<Str name, typename T, std::enable_if_t<is_reflectable<T>, bool> = true>
        constexpr size_t get_index_c() {
            constexpr auto it = std::find(T::member_names.begin(), T::member_names.end(), name.sv());
            constexpr size_t index = std::distance(T::member_names.begin(), it);
            static_assert(index != T::member_names.size(), "You tried to use a name that doesn't belong to any type");
            return index;
        }
        template<typename T, std::enable_if_t<is_reflectable<T>, bool> = true>
        size_t get_index(str name) {
            auto it = std::find(T::member_names.begin(), T::member_names.end(), name);
            size_t index = std::distance(T::member_names.begin(), it);
            if (index == T::member_names.size())
                throw std::runtime_error("[ ERROR ] You tried to use a name that doesn't belong to any type");
            return index;
        }
        
        //: Get member pointer (constexpr, by index or by name)
        template<size_t I, typename T, std::enable_if_t<is_reflectable<T>, bool> = true>
        constexpr auto get_member_i(T* t) {
            constexpr size_t offset = get_offset_c<T, I>();
            using M = std::variant_alternative_t<I, as_type_list<T>>;
            return (M*)(t + offset);
        }
        template<Str name, typename T, std::enable_if_t<is_reflectable<T>, bool> = true>
        constexpr auto get_member(T* t) {
            return get_member_i<get_index_c<name, T>(), T>(t);
        }
        
        //: Apply a function to a member variable by name (runtime)
        template<typename T, typename F, std::enable_if_t<is_reflectable<T>, bool> = true>
        auto apply(T* t, str name, F func) {
            size_t index = get_index<T>(name);
            for_<as_type_list<T>>([&](auto i){
                if (i.value == index) {
                    using M = std::variant_alternative_t<i.value, as_type_list<T>>;
                    M* member = get_member_i<i.value, T>(t);
                    func(member);
                }
            });
        }
    }
}

//: Alternative to Reflection::Members<>
//      struct Test {
//          Members(Test, member)
//          int member;
//      };
//: This is the recommended way to add reflection to VertexData since it doesn't create inheritance, which makes it harder to use the default constructor
#define Members(Type, ...) \
static constexpr std::array<char, ::Fresa::Reflection::str_size(#__VA_ARGS__)> member_name_data = [](){ \
    std::array<char, ::Fresa::Reflection::str_size(#__VA_ARGS__)> chars{'\0'}; \
    size_t _idx = 0; \
    constexpr auto* ini(#__VA_ARGS__); \
    for (char const* _c = ini; *_c; ++_c, ++_idx) \
        if(*_c != ',' && *_c != ' ') \
            chars[_idx] = *_c; \
    return chars;}(); \
\
static constexpr std::array<const char*, ::Fresa::Reflection::n_args(#__VA_ARGS__)> member_names = [](){ \
    std::array<const char*, ::Fresa::Reflection::n_args(#__VA_ARGS__)> out{ }; \
    for(size_t i = 0, n_args = 0; n_args < ::Fresa::Reflection::n_args(#__VA_ARGS__) ; ++i) { \
        while(Type::member_name_data[i] == '\0') i++; \
        out[n_args++] = &Type::member_name_data[i]; \
        while(Type::member_name_data[++i] != '\0'); } \
    return out;}(); \

#endif
