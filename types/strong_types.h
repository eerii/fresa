//* strong type definitions
//      this allows to define aliased types that are different than the base type, but still have the same behavior
//      based on the strong_type library (https://github.com/rollbear/strong_type) by rollbear
#pragma once

#include "std_types.h"

namespace fresa
{
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

    //* concepts
    //      helper concepts to define strong types as well as extra concepts for types that std does not cover
    namespace concepts
    {
        //: is specialization of a base template
        namespace detail
        {
            template <template <typename...> typename Template, typename... Args>
            void DerivedFromImpl(const Template<Args...>&);
        }
        template <typename T, template <typename...> typename Template>
        concept SpecializationOf = requires(const T& t) {
            detail::DerivedFromImpl<Template>(t);
        };

        //: detects if a type is strongly defined
        template <typename T> concept StrongType = SpecializationOf<T, strong::Type>;

        //: decrementable type (similar to std::incrementable)
        template <typename T>
        concept WeaklyDecrementable =
            std::movable<T> and
            requires(T t) {
                typename std::iter_difference_t<T>;
                { --t } -> std::same_as<T&>;
                t--;
            };
        template <typename T>
        concept Decrementable =
            std::regular<T> and
            WeaklyDecrementable<T> and
            requires(T t) {
                { t-- } -> std::same_as<T>;
            };
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
        struct Equality { 
            template <StrongType T> requires std::equality_comparable<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr bool operator==(const T& a, const T& b) noexcept { return a.value == b.value; }
                [[nodiscard]] friend constexpr bool operator!=(const T& a, const T& b) noexcept { return a.value != b.value; }
            };
        };

        //: equality with (between two compatible types, strong or not)
        namespace detail
        {
            //: get underlying value of a expression, either a strong type or a regular one
            template <StrongType T> constexpr auto&& get(T&& t) noexcept { return std::forward<T>(t).value; }
            template <typename T> constexpr T&& get(T&& t) noexcept { return std::forward<T>(t); }

            //: equality with implementation
            template <StrongType A, typename B>
            requires (std::equality_comparable_with<underlying_t<A>, underlying_t<B>> and not std::same_as<A, B>)
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
        struct Ordered {
            template <StrongType T> requires std::totally_ordered<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr bool operator<(const T& a, const T& b) noexcept { return a.value < b.value; }
                [[nodiscard]] friend constexpr bool operator>(const T& a, const T& b) noexcept { return a.value > b.value; }
                [[nodiscard]] friend constexpr bool operator<=(const T& a, const T& b) noexcept { return a.value <= b.value; }
                [[nodiscard]] friend constexpr bool operator>=(const T& a, const T& b) noexcept { return a.value >= b.value; }
            };
        };

        //: ordered with
        namespace detail
        {
            template <StrongType A, typename B>
            requires (std::totally_ordered_with<underlying_t<A>, underlying_t<B>> and not std::same_as<A, B>)
            struct ordered_with {
                [[nodiscard]] friend constexpr bool operator<(const A& a, const B& b) noexcept { return a.value < get(b); }
                [[nodiscard]] friend constexpr bool operator>(const A& a, const B& b) noexcept { return a.value > get(b); }
                [[nodiscard]] friend constexpr bool operator<=(const A& a, const B& b) noexcept { return a.value <= get(b); }
                [[nodiscard]] friend constexpr bool operator>=(const A& a, const B& b) noexcept { return a.value >= get(b); }
                [[nodiscard]] friend constexpr bool operator<(const B& b, const A& a) noexcept { return get(b) < a.value; }
                [[nodiscard]] friend constexpr bool operator>(const B& b, const A& a) noexcept { return get(b) > a.value; }
                [[nodiscard]] friend constexpr bool operator<=(const B& b, const A& a) noexcept { return get(b) <= a.value; }
                [[nodiscard]] friend constexpr bool operator>=(const B& b, const A& a) noexcept { return get(b) >= a.value; }
            };
        }

        template <typename ... Ts>
        struct OrderedWith { template <StrongType T> struct Modifier : detail::ordered_with<T, Ts>... {}; };

        //: semiregular
        struct Semiregular { template <StrongType T> requires std::semiregular<underlying_t<T>> struct Modifier {}; };

        //: regular
        struct Regular { template <StrongType T> struct Modifier : Semiregular::Modifier<T>, Equality::Modifier<T> {}; };

        //: unique (no copy, just move)
        struct Unique {
            template <StrongType T> requires std::movable<underlying_t<T>> struct Modifier {
                constexpr Modifier() = default;
                Modifier(const Modifier &) = delete;
                Modifier &operator=(const Modifier &) = delete;
                constexpr Modifier(Modifier &&) = default;
                constexpr Modifier &operator=(Modifier &&) = default;
            };
        };

        //: incrementable
        namespace detail
        {
            struct OnlyIncrementable {
                template <StrongType T> requires std::incrementable<underlying_t<T>> struct Modifier {
                    friend constexpr T operator++(T& a, int) noexcept { auto r = a; a.value++; return r; }
                    friend constexpr T operator++(T& a) noexcept { a.value++; return a; }
                };
            };
            struct OnlyDecrementable {
                template <StrongType T> requires concepts::Decrementable<underlying_t<T>> struct Modifier {
                    friend constexpr T operator--(T& a, int) noexcept { auto r = a; a.value--; return r; }
                    friend constexpr T operator--(T& a) noexcept { a.value--; return a; }
                };
            };
        }
        struct Incrementable {
            template <StrongType T> struct Modifier : detail::OnlyIncrementable::Modifier<T>, detail::OnlyDecrementable::Modifier<T> {};
        };

        //: boolean
        struct Boolean {
            template <StrongType T> requires std::convertible_to<underlying_t<T>, bool> struct Modifier {
                explicit constexpr operator bool() const noexcept { 
                    const auto &self = (const T&)(*this);
                    return (bool)self.value;
                }
            };
        };
    }
}