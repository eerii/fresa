# [`reflection`](https://github.com/josekoalas/fresa/blob/main/reflection)

Implementing reflection into **fresa** has been one of the most iterative and extensive tasks in the engine. Reflection in c++ as of now is pretty much absent, and there have been a multitude of hacks to get it kind of working. In **fresa**, I want reflection to be pretty much invisible to the user, so they don't have to write any extra code, but also to leverage the power of compile time reflection. These two requirements combined make it very difficult to implement.

I finally managed to get something working that I am relatively happy with, at least until reflection is not propperly implemented in the standard. It works with two components:

- A base that works natively in the language, based on the [pfr](https://github.com/apolukhin/pfr_non_boost) library. It reflects struct members and allows for iteration over them.
- An extension tool that generates extra information that is optional but useful for debugging.

## base functionality

- [`struct_fields.h`](https://github.com/josekoalas/fresa/blob/main/reflection/struct_fields.h): This detects how many fields a struct has, which can be accessed by calling `fresa::field_count<T>`. It is the base of all reflection.
- [`tie_as_tuple.h`](https://github.com/josekoalas/fresa/blob/main/reflection/tie_as_tuple.h): This uses structured bindings to create a tuple from a struct. Since no varadic s.b. are supported in c++, this file is generated up to N struct fields using the `tie_as_tuple_gen.py` script. The default is 32 fields, which should be plenty, but if you need more, you can use the script to generate them.
- [`reflection.h`](https://github.com/josekoalas/fresa/blob/main/reflection/reflection.h): Using the previous implementations, this is the main reflection header that implements all the useful functionality that reflection provides. These range from for each element to custom operators.

### get a field

You can get a member field of a struct with an index like so:

```cpp
struct Position {
    float x;
    float y;
};

Position pos{1.0f, 2.0f};

auto& x = fresa::get<0>(pos);
```

### iterate over fields

```cpp
struct Something {
    str a;
    double b;
    std::vector<float> c;
};

Something s{"hey", 1.0, {1.f, 2.f, 3.f}};

for_([](auto i){
    log::info("{}", i);
}, s);

// >> hey
// >> 1.0
// >> [1.0 2.0 3.0]
```

### custom operators

For every reflectable struct, automatic operators are defined for equality, hash and fmt::format. Note: these operators are under the namespace _fresa_.

```cpp
struct Position {
    float x;
    float y;
};

Position p1{1.0f, 2.0f};
Position p2{3.0f, 2.0f};

log::info("{}", p1 == p2);
// >> false

log::info("{}", p1 != p2);
// >> true

log::info("{}", std::hash<Position>{}(p1)); //: hash means it can be used in an unordered_map
// >> 2139095040

log::info("{}", p1);
// >> {1.0 2.0}
```

## extension tool

The main thing missing from the previous implementation are the member names of each field of the struct. These are really useful, for example, to show variable names in the entity inspector or for readable serialization. However, since there is no standard way of getting these without having the user to type them for each struct (which would mean writting the names twice and having to mantain a separate list each time the struct changes), I decided to implement a tool that generates these names.

I made using the tool optional, so all functions that depend on the member names must have a fallback in case they are not implemented, and the engine should work without them just fine, so no crucial code can be dependent on them existing. The tool in question is [`reflection_gen.py`](https://github.com/josekoalas/fresa/blob/main/reflection/reflection_gen.py), and it is integrated into the CMake build system so each time a reflectable file is changed, the tool runs.

What it does is for each `some_file.h` that is marked as reflectable, it generates a `reflection_some_file.h` file that contains a spetialization of the `field_names<T>()` function that returns a constexpr array of string views with the field names. By default this array is of size zero.

To mark a file as reflectable, simply import the generated file at the end. This might be counter intuitive, but before the code is compiled, the script will generate the code and then "paste" it into the original file, so no error should occur. The name of the generated file will always be `reflection_<filename>.h`.

```cpp title="some_file.cpp"
struct Something {
    str a;
    double b;
    std::vector<float> c;
};

#include "reflection_some_file.h"
```

Then the generated code will produce something like this:

```cpp title="reflection_some_file.h"
//...
#include "reflection.h"
template <> constexpr auto fresa::field_names<Something>() { return std::array<str_view, 3>{"a", "b", "c"}; }
```

This file is non meant to be modified by the user, or imported elsewhere. The reflection tool will scan all structs in the file and write spetializations of the `field_names<T>()` function for each one of them. The import must be done at the end of the file, and can optionally have import guards arround it, like `#if __has_include("reflection_some_file.h")`.