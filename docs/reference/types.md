# [`custom types`](https://github.com/josekoalas/fresa/blob/main/types)

## [`standard types`](https://github.com/josekoalas/fresa/blob/main/types/std_types.h)

Defines standard library imports and useful aliases. Also includes feature testing for the latest `c++20` features, which in case they are not available it replaces them with compatible libraries.

### standard library features

Features from the standard library will be added with a clear intent, trying to keep them as organized as possible. All imports used will be listed in this file.

**math**

- `<cstdint>` for standard integer types
- `<random>` for generating pseudo-random numbers
- `<numbers>` for mathematical constants, including pi
- `std::clamp`

**strings**

- `std::string` (aliased as `str`)
- `std::string_view` (aliased as str_view)

**containers**

_common:_

- `std::array` as the main container, use wherever possible
- `std::vector` only for containers that must be variable, preallocation is recommended
- `std::unordered_map` for key-value pairs
- `std::map` only when ordered access is required
- `<ranges>` [`c++20`] for algorithms and iteration

_specific:_

- `std::deque` for lists where a lot of elements are added to or removed at the back
- `std::set` for unique identifier lists, has useful mathematical properties
- `std::stack` for LIFO containers, for example, system initialization/destruction
- `std::queue` and `std::priority_queue` for other ordered containers

**metaprograming**

- templates
- lambdas (and templated lambdas [`c++20`])
- concepts [`c++20`]
- `std::optional`
- `<tuple>`

**compiler**

- feature testing [`c++20`]
- `std::source_location` [`c++20`] for unit testing (alternative implementation provided)

**time**

- `std::chrono` for system independent timers

**concurrency**

- coroutines [`c++20`]
- `std::jthread` [`c++20`] for multithreading
- `std::atomic`, `std::mutex` and `std::condition_variable` for thread syncronization

### aliases

Some standard types are aliased to improve on readability and code clutter. Also, this is made to avoid using the `std` namespace, as that can bring confusion and errors. I tried to use common and easy to understand aliases, and a table is available with all conversions:

| type | alias |
|---|---|
| `std::uint8_t` | `ui8` |
| `std::uint16_t` | `ui16` |
| `std::uint32_t` | `ui32` |
| `std::uint64_t` | `ui64` |
| `std::string` | `str` |
| `std::string_view` | `str_view` |

The namespace aliases are mainly to allow compatibility between the standard library and helper libraries while compilers get support of `c++20`. For example, the `range-v3` library defines everything in the namespace `ranges`, while the standard library uses `std::ranges`. Similarly, in `clang` coroutine features are under `std::experimental`, while on `gcc` they are under `std`. Once the libraries are homogeneous this aliases might be removed.

| namespace | alias |
|---|---|
| `std::ranges` | `ranges` |
| `std::ranges::views` | `rv` |
| `std::experimental` | `std_` |

## [`type name`](https://github.com/josekoalas/fresa/blob/main/types/type_name.h)

Constexpr compiler-independent type name implementation.

```cpp
constexpr str_view fresa::type_name_n<TYPE>();
constexpr str_view fresa::type_name<TYPE>();
```

There are two versions of the function. `type_name_n` returns the full type including all namespaces, while `type_name` removes `fresa` namespaces and only returns the name of the type.

```cpp
type_name_n<fresa::detail::array>() == "fresa::detail::array";
type_name<fresa::detail::array>() == "array";
//: while
type_name_n<std::array>() == "std::array";
type_name<std::array>() == "std::array";
```

## [`constexpr for`](https://github.com/josekoalas/fresa/blob/main/types/constexpr_for.h)

Approach of a variety of constexpr for loops. Uses template metaprograming to create the iteration on compile time, supporting constexpr results.

**integral for**

Similar to a regular for loop, has a range of integral values [a, b) and calls the function inside each time with a different value of `i`. In the case of the constexpr for, `i` is an integral constant which can be used as a constexpr, for example for template parameters.

```cpp
//: regular for
for (int i = 0; i < 5; i++) { ... }

//: constexpr for
for_<0, 5>([&](auto i){
    ...
});

//: also supports a custom increment
for_<0, 30, 3>([](auto i){ ... });
```

**parameter pack for**

Recursively calls the function with each of the elements provided. The values can be heterogeneous.

```cpp
for_([&](auto a){
    ...
}, 2, 4, 8, 16);
```

**tuple like for**

Very similar to the previous one, but perhaps more useful. Iterates over the values of a tuple like object, much like a range based for loop.

```cpp
//: range based for loop
for (auto a : some_array) { ... }

//: constexpr tuple-like for loop
for_([&](auto const& a){
    ...
}, std::make_tuple(true, 1, "hello"));
```

## [`string utils`](https://github.com/josekoalas/fresa/blob/main/types/strings.h)

**string literal**

Used for passing a string literal as a template parameter.

```cpp
template<str_literal name>
void function() {
    name.value;
}

function<"hey">();
```

**lowercase**

Returns a lowercase string view. Works for constexpr strings. There is also a literal operator for in place conversion.

```cpp
fresa::lower("FrEsA") == "fresa"
"MeRmElAdA"_lower == "mermelada"
```

**split**

Parses a string view and returns a range of elements separated by the delimiter (by default a space). The returned object is a range, but can be easily convertible to a vector using `ranges::to_vector`.

```cpp
auto s_range = split("a,b,c,d", ',');
auto s_vector = s_range | ranges::to_vector;

//: removes extra delimiters
auto s = split("a  b c   d") | ranges::to_vector == {"a", "b", "c", "d"};
```

## [`atomic queue`](https://github.com/josekoalas/fresa/blob/main/types/atomic_queue.h)

**spin lock**

Simple implementation of a spin lock using `std::atomic_flag` for thread syncronization. Can be used inside `std::lock_guard`.

```cpp
fresa::SpinLock lock;
//...
{
    std::lock_guard<SpinLock> guard(lock);
    //: thread safe code
}
```

**atomic queue**

Adaptation of `std::queue` to be thread safe using `SpinLock`. Can't be copied, only moved (for example, to emplace in a vector or atomic queues). One of its main usages is for the [job system](jobs.md)'s worker queues.

```cpp
fresa::AtomicQueue<T> queue;
//: add to the end of the queue
queue.push(T{});
//: get an item from the front of the queue and remove it
//  if the queue is empty, return an empty optional
std::optional<T> t = queue.pop();
//: elements left in the queue
std::size_t n = queue.size();
//: clear the queue
queue.clear();
```

## [`coroutines`](https://github.com/josekoalas/fresa/blob/main/types/coroutines.h)

See [coroutines](coroutines.md).

## [`strong types`](https://github.com/josekoalas/fresa/blob/main/types/strong_types.h)

A reimplementation of rollbear's [strong type](https://github.com/rollbear/strong_type) library. The functionality and usage is mostly the same, but I simplified it a bit for our usecase and used concepts instead of SFINAE for clarity since `c++20` is already required for the rest of the engine. All credits go to their implementation.

To describe a strong type alias you do the following:

```cpp
#include "strong_types.h"
using IntLike = strong::Type<int, decltype([]{}), strong::Regular, ...>;
```

The first parameter of `strong::Type` is the type you are aliasing. The second is a tag, which is an unique template parameter to keep different strong types distinct. We can use the lambda return type for this since it is always guaranteed to be unique, written as `decltype([]{})`. Finally, a list of modifiers can be optionally specified. These are passthroughs added to the strong type to allow easier manipulation and functionality.

**modifiers**

- `Equality` and `EqualityWith<T>` provide equality operators (`==` and `!=`) with the same type and with another type `T` respectively. This extra type can be strong or regular, but can't be the same type you are defining (for that you can use the regular `Equality`).
- `Ordered` and `OrderedWith<T>` provide comparison operators (`<`, `>`, `<=` and `>=`).
- `Semiregular` is default constructible, move and copy constructible, move and copy assignable and swappable.
- `Regular` extends all semiregular requirements while also being equality comparable. Recommended base type.
- `Unique` makes the type move constructible and assignable but not copy constructible or assignable.
- `Incrementable` provides increment operators (`++` and `--`). For exclusive increment or decrement use `detail::OnlyIncrementable` and `detail::OnlyDecrementable`.
- `Boolean` gives the type a cast to `bool`, for example, to use inside if statements.
- `OStreamable`, `IStreamable` and `IOStreamable` pass the stream operators (`<<` and `>>`) to the base type.
- `Arithmetic` and `ArithmeticWith<T>` provide arithmetic operators (`+`, `-`, `*`, `/` and `%`), as well as the corresponding assignment operators (`+=`, `-=`, `*=`, `/=` and `%=`).
- `Bitwise` and `BitwiseWith<T>` provide bitwise operators (`&`, `|`, `^`, `<<` and `>>`), as well as the corresponding assigment operators (`&=`, `|=`, `^=`, `<<=` and `>>=`).
- `Indexed` allows to access the base type indices using `[]` and `.at()`.
- `Iterator` adds functionality depending on the iterator type.
- `Range` allows to iterate over the elements. Defines `begin` and `end`, as well as `cbegin` and `cend`. You can use the type inside a range based for loop.
- `ConvertibleTo<T>` gives the type an explicit cast to `T`. `ImplicitlyConvertibleTo<T>` gives it an implicit cast. Use the latter with caution since it can defeat the whole point of having a separate strong type.
- `Hashable<T>` spetializes `std::hash` to take this strong type. Allows to use it as a key in an `std::unordered_map`. Inherits `Equality`.
- `Formattable<T>` spetializes `fmt::format` if the library is available. Allows to use `log` to print it.