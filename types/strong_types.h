//* strong type definitions
//      this allows to define aliased types that are different than the base type, but still have the same behavior
//      based on the strong_type library (https://github.com/rollbear/strong_type) by rollbear but simplified for our use case and using concepts
//      should have similar performance as the original type since all defined members are constant expressions passthroughs to the base type
#pragma once

#include "std_types.h"

//* example usage
//      using IntLike = strong::Type<int, decltype([]{}), strong::Regular, strong::Arithmetic>;

//* available modifiers
//      : equality / equality with (==, !=)
//      : ordered / ordered with (<, >, <=, >=)
//      : semiregular
//      : regular
//      : unique
//      : incrementable (++, --)
//      : boolean
//      : io streamable (<<, >>)
//      : arithmetic / arithmetic with (+, -, *, /, %, +=...)
//      : bitwise / bitwise with (&, |, ^, <<, >>, &=...)
//      : indexed ([], at)
//      : iterator
//      : range (begin, end)
//      : convertible to / implicitly convertible to
//      : hashable
//      : formattable

namespace fresa
{
    namespace strong
    {
        //* type modifiers
        template <typename M, typename T>
        using Modifier = typename M::template Modifier<T>;

        //* strong type implementation
        template <typename T, typename Tag = decltype([]{}), typename ... M>
        struct Type : Modifier<M, Type<T, Tag, M...>>... {
            //: constructor with default value
            constexpr Type() noexcept requires std::default_initializable<T> : value{} {}
            //: constructor using initializer list
            template <typename U> constexpr Type(std::initializer_list<U> list) noexcept requires std::constructible_from<T, std::initializer_list<U>> : value(list) {}
            //: constructor using arguments and forwarding
            template <typename ... U> constexpr Type(U&& ...args) noexcept requires (std::constructible_from<T, U&&...> and sizeof...(U) > 0) : value(std::forward<U>(args)...) {}

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
            template <template <auto...> typename Template, auto... Args>
            void DerivedFromImplValue(const Template<Args...>&);
        }
        template <typename T, template <typename...> typename Template>
        concept SpecializationOf = requires(const T& t) {
            detail::DerivedFromImpl<Template>(t);
        };
        template <typename T, template <auto...> typename Template>
        concept SpecializationOfV = requires(const T& t) {
            detail::DerivedFromImplValue<Template>(t);
        };

        //: detects if a type is strongly defined
        template <typename T> concept StrongType = SpecializationOf<T, strong::Type>;

        //: decrementable type (similar to std::incrementable)
        template <typename T>
        concept WeaklyDecrementable = std::movable<T> and requires(T t) {
            typename std::iter_difference_t<T>;
            { --t } -> std::same_as<T&>;
            t--;
        };
        template <typename T>
        concept Decrementable = std::regular<T> and WeaklyDecrementable<T> and requires(T t) {
            { t-- } -> std::same_as<T>;
        };

        //: i/o streamable type
        template <typename T>
        concept OStreamable = requires(std::ostream& os, const T& t) { os << t; };
        template <typename T>
        concept IStreamable = requires(std::istream& is, T& t) { is >> t; };
        template <typename T>
        concept IOStreamable = OStreamable<T> and IStreamable<T>;
    }

    namespace strong
    {
        using namespace ::fresa::concepts;

        //* underlying type (the type that is being wrapped)
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
        //      extra parameter to add to a strong type that modifies its behavior with standard properties
        //      the difference between "modifier" and "modifier with" is that the second one is with another type, which can be strong or not

        //: equality (==, !=)
        struct Equality { 
            template <StrongType T> requires std::equality_comparable<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr bool operator==(const T& a, const T& b) noexcept { return a.value == b.value; }
                [[nodiscard]] friend constexpr bool operator!=(const T& a, const T& b) noexcept { return a.value != b.value; }
            };
        };

        //: equality with (==, !=)
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

        //: ordered (<, >, <=, >=)
        struct Ordered {
            template <StrongType T> requires std::totally_ordered<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr bool operator<(const T& a, const T& b) noexcept { return a.value < b.value; }
                [[nodiscard]] friend constexpr bool operator>(const T& a, const T& b) noexcept { return a.value > b.value; }
                [[nodiscard]] friend constexpr bool operator<=(const T& a, const T& b) noexcept { return a.value <= b.value; }
                [[nodiscard]] friend constexpr bool operator>=(const T& a, const T& b) noexcept { return a.value >= b.value; }
            };
        };

        //: ordered with (<, >, <=, >=)
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

        //: semiregular (default constructible, move/copy constructible, move/copy assignable and swappable)
        struct Semiregular { template <StrongType T> requires std::semiregular<underlying_t<T>> struct Modifier {}; };

        //: regular (semiregular and equality comparable, best base modifier for most types)
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

        //: incrementable (++, --)
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

        //: boolean (directly used in if statements or similar)
        struct Boolean {
            template <StrongType T> requires std::convertible_to<underlying_t<T>, bool> struct Modifier {
                explicit constexpr operator bool() const noexcept { 
                    const auto &self = (const T&)(*this);
                    return (bool)self.value;
                }
            };
        };

        //: io stream
        struct OStreamable {
            template <StrongType T> requires concepts::OStreamable<underlying_t<T>> struct Modifier {
                friend std::ostream& operator<<(std::ostream& os, const T& a) { return os << a.value; }
            };
        };
        struct IStreamable {
            template <StrongType T> requires concepts::IStreamable<underlying_t<T>> struct Modifier {
                friend std::istream& operator>>(std::istream& is, T& a) { return is >> a.value; }
            };
        };
        struct IOStreamable {
            template <StrongType T> struct Modifier : OStreamable::Modifier<T>, IStreamable::Modifier<T> {};
        };

        //: arithmetic (+, -, *, /, %)
        struct Arithmetic {
            template <StrongType T> requires std::is_arithmetic_v<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr T operator+(const T& a, const T& b) noexcept { return a.value + b.value; }
                [[nodiscard]] friend constexpr T operator-(const T& a, const T& b) noexcept { return a.value - b.value; }
                [[nodiscard]] friend constexpr T operator*(const T& a, const T& b) noexcept { return a.value * b.value; }
                [[nodiscard]] friend constexpr T operator/(const T& a, const T& b) noexcept { return a.value / b.value; }
                [[nodiscard]] friend constexpr T operator%(const T& a, const T& b) noexcept requires std::integral<underlying_t<T>> { return a.value % b.value; }
                friend constexpr T operator+=(T& a, const T& b) noexcept { a.value += b.value; return a; }
                friend constexpr T operator-=(T& a, const T& b) noexcept { a.value -= b.value; return a; }
                friend constexpr T operator*=(T& a, const T& b) noexcept { a.value *= b.value; return a; }
                friend constexpr T operator/=(T& a, const T& b) noexcept { a.value /= b.value; return a; }
                friend constexpr T operator%=(T& a, const T& b) noexcept requires std::integral<underlying_t<T>> { a.value %= b.value; return a; }
            };
        };

        //: arithmetic with (+, -, *, /, %)
        namespace detail
        {
            template <StrongType A, typename B>
            requires (std::is_arithmetic_v<underlying_t<A>> and std::is_arithmetic_v<underlying_t<B>> and not std::same_as<A, B>)
            struct arithmetic_with {
                [[nodiscard]] friend constexpr A operator+(const A& a, const B& b) noexcept { return a.value + get(b); }
                [[nodiscard]] friend constexpr A operator-(const A& a, const B& b) noexcept { return a.value - get(b); }
                [[nodiscard]] friend constexpr A operator*(const A& a, const B& b) noexcept { return a.value * get(b); }
                [[nodiscard]] friend constexpr A operator/(const A& a, const B& b) noexcept { return a.value / get(b); }
                
                friend constexpr A operator+=(A& a, const B& b) noexcept { a.value += get(b); return a; }
                friend constexpr A operator-=(A& a, const B& b) noexcept { a.value -= get(b); return a; }
                friend constexpr A operator*=(A& a, const B& b) noexcept { a.value *= get(b); return a; }
                friend constexpr A operator/=(A& a, const B& b) noexcept { a.value /= get(b); return a; }
                
                [[nodiscard]] friend constexpr A operator+(const B& b, const A& a) noexcept { return get(b) + a.value; }
                [[nodiscard]] friend constexpr A operator-(const B& b, const A& a) noexcept { return get(b) - a.value; }
                [[nodiscard]] friend constexpr A operator*(const B& b, const A& a) noexcept { return get(b) * a.value; }
                [[nodiscard]] friend constexpr A operator/(const B& b, const A& a) noexcept { return get(b) / a.value; }

                [[nodiscard]] friend constexpr A operator%(const A& a, const B& b) noexcept
                requires std::integral<underlying_t<A>> and std::integral<underlying_t<B>> { return a.value % get(b); }
                friend constexpr A operator%=(A& a, const B& b) noexcept
                requires std::integral<underlying_t<A>> and std::integral<underlying_t<B>> { a.value %= get(b); return a; }
                [[nodiscard]] friend constexpr A operator%(const B& b, const A& a) noexcept
                requires std::integral<underlying_t<A>> and std::integral<underlying_t<B>> { return get(b) % a.value; }
            };
        }
        template <typename ... Ts>
        struct ArithmeticWith { template <StrongType T> struct Modifier : detail::arithmetic_with<T, Ts>... {}; };

        //: bitwise (<<, >>, &, |, ^)
        struct Bitwise {
            template <StrongType T> requires std::integral<underlying_t<T>> struct Modifier {
                [[nodiscard]] friend constexpr T operator<<(const T& a, const T& b) noexcept { return a.value << b.value; }
                [[nodiscard]] friend constexpr T operator>>(const T& a, const T& b) noexcept { return a.value >> b.value; }
                [[nodiscard]] friend constexpr T operator&(const T& a, const T& b) noexcept { return a.value & b.value; }
                [[nodiscard]] friend constexpr T operator|(const T& a, const T& b) noexcept { return a.value | b.value; }
                [[nodiscard]] friend constexpr T operator^(const T& a, const T& b) noexcept { return a.value ^ b.value; }
                friend constexpr T operator<<=(T& a, const T& b) noexcept { a.value <<= b.value; return a; }
                friend constexpr T operator>>=(T& a, const T& b) noexcept { a.value >>= b.value; return a; }
                friend constexpr T operator&=(T& a, const T& b) noexcept { a.value &= b.value; return a; }
                friend constexpr T operator|=(T& a, const T& b) noexcept { a.value |= b.value; return a; }
                friend constexpr T operator^=(T& a, const T& b) noexcept { a.value ^= b.value; return a; }
            };
        };

        //: bitwise with (<<, >>, &, |, ^)
        namespace detail
        {
            template <StrongType A, typename B>
            requires (std::integral<underlying_t<A>> and std::integral<underlying_t<B>> and not std::same_as<A, B>)
            struct bitwise_with {
                [[nodiscard]] friend constexpr A operator<<(const A& a, const B& b) noexcept { return a.value << get(b); }
                [[nodiscard]] friend constexpr A operator>>(const A& a, const B& b) noexcept { return a.value >> get(b); }
                [[nodiscard]] friend constexpr A operator&(const A& a, const B& b) noexcept { return a.value & get(b); }
                [[nodiscard]] friend constexpr A operator|(const A& a, const B& b) noexcept { return a.value | get(b); }
                [[nodiscard]] friend constexpr A operator^(const A& a, const B& b) noexcept { return a.value ^ get(b); }

                friend constexpr A operator<<=(A& a, const B& b) noexcept { a.value <<= get(b); return a; }
                friend constexpr A operator>>=(A& a, const B& b) noexcept { a.value >>= get(b); return a; }
                friend constexpr A operator&=(A& a, const B& b) noexcept { a.value &= get(b); return a; }
                friend constexpr A operator|=(A& a, const B& b) noexcept { a.value |= get(b); return a; }
                friend constexpr A operator^=(A& a, const B& b) noexcept { a.value ^= get(b); return a; }
                
                [[nodiscard]] friend constexpr A operator<<(const B& b, const A& a) noexcept { return get(b) << a.value; }
                [[nodiscard]] friend constexpr A operator>>(const B& b, const A& a) noexcept { return get(b) >> a.value; }
                [[nodiscard]] friend constexpr A operator&(const B& b, const A& a) noexcept { return get(b) & a.value; }
                [[nodiscard]] friend constexpr A operator|(const B& b, const A& a) noexcept { return get(b) | a.value; }
                [[nodiscard]] friend constexpr A operator^(const B& b, const A& a) noexcept { return get(b) ^ a.value; }
            };
        }
        template <typename ... Ts>
        struct BitwiseWith { template <StrongType T> struct Modifier : detail::bitwise_with<T, Ts>... {}; };

        //: pointer (*, ->)
        struct Pointer {
            template <StrongType T> struct Modifier {
                [[nodiscard]] friend constexpr bool operator==(const T& t, std::nullptr_t) noexcept { return t.value == nullptr; }
                [[nodiscard]] friend constexpr bool operator!=(const T& t, std::nullptr_t) noexcept { return t.value != nullptr; }
                [[nodiscard]] friend constexpr bool operator==(std::nullptr_t, const T& t) noexcept { return t.value == nullptr; }
                [[nodiscard]] friend constexpr bool operator!=(std::nullptr_t, const T& t) noexcept { return t.value != nullptr; }

                [[nodiscard]] constexpr auto operator*() const -> decltype(*std::declval<const underlying_t<T>&>()) {
                    auto &self = (const T&)(*this);
                    return *self.value;
                }
                [[nodiscard]] constexpr auto operator->() const -> decltype(&(*std::declval<const underlying_t<T>&>())) { &operator*(); }
            };
        };

        //: indexed ([], at)
        struct Indexed {
            template <StrongType T> struct Modifier {
                template <typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto operator[](const I& i) const& noexcept -> decltype(std::declval<const underlying_t<T>&>()[detail::get(i)]) {
                    const auto &self = (const T&)(*this);
                    return self.value[detail::get(i)];
                }
                template <typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto operator[](const I& i) & noexcept -> decltype(std::declval<underlying_t<T>&>()[detail::get(i)]) {
                    auto &self = (T&)(*this);
                    return self.value[detail::get(i)];
                }
                template <typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto operator[](const I& i) && noexcept -> decltype(std::declval<underlying_t<T>&&>()[detail::get(i)]) {
                    auto &self = (T&)(*this);
                    return std::move(self).value[detail::get(i)];
                }
                template <typename TT = underlying_t<T>, typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto at(const I& i) const& noexcept -> decltype(std::declval<const TT&>().at(detail::get(i))) {
                    const auto &self = (const T&)(*this);
                    return self.value.at(detail::get(i));
                }
                template <typename TT = underlying_t<T>, typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto at(const I& i) & noexcept -> decltype(std::declval<TT&>().at(detail::get(i))) {
                    auto &self = (T&)(*this);
                    return self.value.at(detail::get(i));
                }
                template <typename TT = underlying_t<T>, typename I> requires std::integral<underlying_t<I>>
                [[nodiscard]] constexpr auto at(const I& i) && noexcept -> decltype(std::declval<TT&&>().at(detail::get(i))) {
                    auto &self = (T&)(*this);
                    return std::move(self).value.at(detail::get(i));
                }
            };
        };

        //: iterator
        struct Iterator {
            template <StrongType T, typename Cat = typename std::iterator_traits<underlying_t<T>>::iterator_category>
            struct Modifier : Equality::Modifier<T>, Pointer::Modifier<T>, detail::OnlyIncrementable::Modifier<T> {
                using difference_type = typename std::iterator_traits<underlying_t<T>>::difference_type;
                using value_type = typename std::iterator_traits<underlying_t<T>>::value_type;
                using pointer = typename std::iterator_traits<underlying_t<T>>::value_type;
                using reference = typename std::iterator_traits<underlying_t<T>>::reference;
                using iterator_category = typename std::iterator_traits<underlying_t<T>>::iterator_category;
            };

            template <StrongType T> struct Modifier<T, std::bidirectional_iterator_tag> : Modifier<T, std::forward_iterator_tag>, detail::OnlyDecrementable::Modifier<T> {};
            template <StrongType T> struct Modifier<T, std::random_access_iterator_tag> : Modifier<T, std::bidirectional_iterator_tag>, Indexed::Modifier<T>, Ordered::Modifier<T> {};
        };

        //: range (begin, end)
        struct Range {
            template <StrongType T> requires ranges::range<underlying_t<T>> struct Modifier;
        };
        template <typename T, typename Tag, typename ... M> struct Range::Modifier<Type<T, Tag, M...>> {
            using ST = Type<T, Tag, M...>;
            using It = Type<decltype(std::declval<underlying_t<T>&>().begin()), Tag, Iterator>;
            using CIt = Type<decltype(std::declval<const underlying_t<T>&>().begin()), Tag, Iterator>;

            It begin() & noexcept { 
                auto &self = (ST&)(*this);
                return It{self.value.begin()};
            }
            CIt begin() const& noexcept {
                const auto &self = (const ST&)(*this);
                return CIt{self.value.begin()};
            }
            CIt cbegin() const& noexcept {
                const auto &self = (const ST&)(*this);
                return CIt{self.value.cbegin()};
            }

            It end() & noexcept { 
                auto &self = (ST&)(*this);
                return It{self.value.end()};
            }
            CIt end() const& noexcept {
                const auto &self = (const ST&)(*this);
                return CIt{self.value.end()};
            }
            CIt cend() const& noexcept {
                const auto &self = (const ST&)(*this);
                return CIt{self.value.cend()};
            }
        };

        //: convertible to
        namespace detail
        {
            template <StrongType A, typename B>
            requires (std::convertible_to<underlying_t<A>, underlying_t<B>> and not std::same_as<A, B>)
            struct convertible_to {
                constexpr explicit operator B() const noexcept { 
                    auto &self = (A&)(*this);
                    return B{underlying_t<B>(self.value)};
                }
            };
        }
        template <typename ... Ts>
        struct ConvertibleTo { template <StrongType T> struct Modifier : detail::convertible_to<T, Ts>... {}; };

        //: implicitly convertible to
        namespace detail
        {
            template <StrongType A, typename B>
            requires (std::convertible_to<underlying_t<A>, underlying_t<B>> and not std::same_as<A, B>)
            struct implicitly_convertible_to {
                constexpr operator B() const noexcept { 
                    auto &self = (A&)(*this);
                    return B{underlying_t<B>(self.value)};
                }
            };
        }
        template <typename ... Ts>
        struct ImplicitlyConvertibleTo { template <StrongType T> struct Modifier : detail::implicitly_convertible_to<T, Ts>... {}; };

        //: hashable
        struct Hashable {
            template <StrongType T> struct Modifier : Equality::Modifier<T> {
                constexpr static bool is_hashable = true;
            };
        };

        //: formattable (implementation inside fmt namespace)
        struct Formattable {
            template <StrongType T> struct Modifier {
                constexpr static bool is_formattable = true;
            };
        };
    }
}

//: std hashable type
namespace std
{
    template <::fresa::concepts::StrongType T> requires T::is_hashable
    struct hash<T> {
        constexpr std::size_t operator()(const T& t) const {
            auto &self = (const T&)(t);
            return std::hash<::fresa::strong::underlying_t<T>>()(self.value);
        }
    };
}

//: fmt formattable type
#if __has_include("fmt/format.h")
    #include "fmt/format.h"

    namespace fresa::detail
    {
        template <concepts::StrongType T> requires T::is_formattable
        struct format_strong_impl : fmt::formatter<strong::underlying_t<T>> {
            template <typename FormatContext> constexpr auto format(const T& c, FormatContext& ctx) noexcept {
                return fmt::formatter<strong::underlying_t<T>>::format(c.value, ctx);
            }
        };
    }

    template <typename T> requires T::is_formattable
    struct fmt::formatter<T> : fresa::detail::format_strong_impl<T> {};
#endif