//* fresa_types
//      combines types imported from the standard library (std_types.h) with custom types created in fresa
//      this header is meant to be used engine-wide, as a level of providing a common base type system
#pragma once

//* standard types
#include "std_types.h"

//* string utilities
#include "strings.h"

namespace fresa
{
    //* type name implementation
    //      this function provides a constexpr string view with the name of a type
    //      type_name_n returns the name of the type including all namespaces
    //      type_name returns the name of the type excluding fresa namespaces
    //      relevant discussion in https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c

    //: forward declaration of type_name_t to allow overloading
    template <typename T> constexpr str_view type_name_n();

    //: type_name_t is preinitialized for void, this way we can automatically
    //  calculate compiler specific prefixes and suffixes and trim the other names accordingly
    template <> constexpr str_view type_name_n<void>() { return "void"; }

    namespace detail
    {
        //: compiler specific type representations
        template <typename T>
        constexpr str_view compiler_type_name() {
            #if defined __clang__
                return __PRETTY_FUNCTION__;
            #elif defined __GNUC__
                return __PRETTY_FUNCTION__;
            #elif defined _MSC_VER
                return __FUNCSIG__;
            #else
                #error "compiler not supported"
            #endif
        }

        //: calculate prefix and suffix length using the precalculated name for void
        using type_probe = void;
        constexpr std::size_t compiler_type_prefix() {
            return compiler_type_name<type_probe>().find(type_name_n<type_probe>());
        }
        constexpr std::size_t compiler_type_suffix() {
            return compiler_type_name<type_probe>().length() - compiler_type_prefix() - type_name_n<type_probe>().length();
        }
    }

    //: constexpr type names including all namespaces
    //  to exclude fresa namespaces, use type_name<T>()
    template <typename T>
    constexpr str_view type_name_n() {
        constexpr auto compiler_name = detail::compiler_type_name<T>();
        return compiler_name.substr(detail::compiler_type_prefix(), compiler_name.length() - detail::compiler_type_prefix() - detail::compiler_type_suffix());
    }

    //: constexpr type names excluding fresa namespaces
    //  for example, "std::vector" will remain the same, but "fresa::graphics::Buffer" will return "Buffer"
    //  to include namespaces use, use type_name_n<T>()
    template <typename T>
    constexpr str_view type_name() {
        constexpr auto name = type_name_n<T>();
        if (name.find("fresa::") != str_view::npos) {
            auto pos = name.find_last_of("::");
            return name.substr(pos + 1);
        }
        return name;
    }

    //* concepts
    //      helper concepts for tuple like objects and template specialization
    namespace concepts
    {
        namespace detail
        {
            template <typename T, std::size_t N>
            concept HasTupleElement = requires(T t) {
                typename std::tuple_element_t<N, std::remove_const_t<T>>;
                { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
            };

            template <template <typename...> typename Template, typename... Args>
            void DerivedFromImpl(const Template<Args...>&);
        }

        //: is tuple like object
        template <typename T>
        concept TupleLike = requires (T t) {
            typename std::tuple_size<T>::type;
        } and []<std::size_t ... N>(std::index_sequence<N...>) {
            return (detail::HasTupleElement<T, N> && ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>());

        //: is specialization of a base template
        template <typename T, template <typename...> typename Template>
        concept SpecializationOf = requires(const T& t) {
            detail::DerivedFromImpl<Template>(t);
        };
    }

    //* constexpr for
    //      this function provides a set of constexpr for loops for iterating using integral constants or parameters
    //      the implementation is derived from this article https://artificial-mind.net/blog/2020/10/31/constexpr-for
    
    //: integral loop (i = 0; i < n; i++)
    template <auto From, auto To, auto Increment = 1, typename F>
    constexpr void for_(F&& f) {
        if constexpr (From < To) {
            using T = decltype(f(std::integral_constant<decltype(From), From>()));
            if constexpr (std::same_as<T, void>) {
                f(std::integral_constant<decltype(From), From>());
            } else if constexpr (std::convertible_to<T, bool>) {
                if (not f(std::integral_constant<decltype(From), From>()))
                    return;
            }
            for_<From + Increment, To, Increment>(f);
        }
    }

    //: parameter pack loop (auto a : args...)
    template <typename F, typename ... A>
    constexpr void for_(F&& f, A&& ... args) {
        (f(std::forward<A>(args)), ...);
    }

    //: tuple like loop (auto a : tuple_object)
    template <typename F, concepts::TupleLike T>
    constexpr void for_(F&& f, T&& t) {
        constexpr std::size_t N = std::tuple_size_v<std::decay_t<T>>;
        for_<0, N>([&](auto i) {
            f(std::get<i>(t));
        });
    }

    //* strong type definitions
    //      this allows to define aliased types that are different than the base type, but still have the same behavior
    //      based on the strong_type library (https://github.com/rollbear/strong_type) by rollbear
    namespace strong
    {
        //: type modifiers
        template <typename M, typename T>
        using Modifier = typename M::template Modifier<T>;

        //: strong type implementation
        template <typename T, typename Tag = decltype([]{}), typename ... M>
        struct Type : Modifier<M, Type<T, Tag, M...>>... {
            //: constructor with default value
            constexpr Type() noexcept requires std::default_initializable<T> : value{} {}
            //: constructor using initializer list
            template <typename U> constexpr Type(std::initializer_list<U> list) noexcept requires std::constructible_from<T, std::initializer_list<U>> : value{list} {}
            //: constructor using arguments and forwarding
            template <typename ... U> constexpr Type(U&& ...args) noexcept requires (std::constructible_from<T, U&&...> and sizeof...(U) > 0) : value{std::forward<U>(args)...} {}
            
            //: swaping two values
            constexpr void swap(Type &a, Type &b) noexcept requires std::swappable<T> { ranges::swap(a.value, b.value); }

            //: value holder
            T value;
        };
    }

    //: detects if a type is strongly defined
    namespace concepts
    {
        template <typename T> concept StrongType = SpecializationOf<T, strong::Type>;
    }

    namespace strong
    {
        using namespace ::fresa::concepts;

        //: underlying type (the type that is being wrapped)
        namespace detail
        {
            template <typename T, typename Tag, typename ... M>
            constexpr T underlying(Type<T, Tag, M...> *);
        }
        template <typename T, bool = StrongType<T>>
        struct UnderlyingType { using type = decltype(detail::underlying((T*)(nullptr))); };
        template <typename T>
        struct UnderlyingType<T, false> { using type = T; };
        template <typename T>
        using underlying_t = typename UnderlyingType<T>::type;

        //* modifiers

        //: equality (between two instances of the same strong type)
        struct Equality { template <StrongType T> struct Modifier; };

        template <typename T, typename Tag, typename... M> requires std::equality_comparable<T>
        struct Equality::Modifier<Type<T, Tag, M...>> {
            using ST = Type<T, Tag, M...>;
            [[nodiscard]] friend constexpr bool operator==(const ST& a, const ST& b) noexcept { return a.value == b.value; }
            [[nodiscard]] friend constexpr bool operator!=(const ST& a, const ST& b) noexcept { return a.value != b.value; }
        };

        //: equality with (between two compatible types, strong or not)
        namespace detail
        {
            template <StrongType T> constexpr auto&& get(T&& t) noexcept { return std::forward<T>(t).value; }
            template <typename T> constexpr T&& get(T&& t) noexcept { return std::forward<T>(t); }

            template <StrongType A, typename B>
            struct equality_with {
                [[nodiscard]] friend constexpr bool operator==(const A& a, const B& b) noexcept { return a.value == get(b); }
                [[nodiscard]] friend constexpr bool operator!=(const A& a, const B& b) noexcept { return a.value != get(b); }
                [[nodiscard]] friend constexpr bool operator==(const B& b, const A& a) noexcept { return a.value == get(b); }
                [[nodiscard]] friend constexpr bool operator!=(const B& b, const A& a) noexcept { return a.value != get(b); }
            };
        }

        template <typename ... Ts>
        struct EqualityWith { template <StrongType T> struct Modifier : detail::equality_with<T, Ts>... {}; };

        //: ordered

        //: ordered with
    }
}

