# coding style

## naming conventions

In **fresa :strawberry:** we will use the following name conventions for the different types of objects:

- **snake_case**: variables, namespaces
- **camelCase**: functions
- **PascalCase**: structs, interfaces
- **SCREAMING_CASE**: constants, macros

```cpp
namespace fresa {
  
  constexpr int NUMBER_OF_OBJECTS = 256;

  struct Object {
    int position_x;
    float velocity_y;
  };

  Object registerObject(int start_position) {}

  #ifdef USE_VULKAN
  using IPipeline = vkPipeline;
  #endif
}
```

Namespaces will always try to be only one word. Interfaces will start with a capital "I". These conventions might be broken when using external libraries, such as `vulkan`.

We will also denote the engine name in fully lowercase letters and accompanied by the strawberry emoji, **fresa :strawberry:**. All titles of documentation sections will be similarly written in lowercase.

## comments and documentation

Documentation will be written for functions and custom structures using a simplified doxygen-like syntax. All the extra fields are optional and will only be added if the function needs them for clarity.

```cpp
/// function description
/// @param float a: some number
/// @param float b: some other number
/// @example path/to/code.cpp
/// @return int: rounded sum of a and b
int someFunction(float a, float b) {
  return (int)std::round(a+b)
}
```

Other types of comments include:

```cpp
//: regular comments
//- todos
//? implementation questions
//! alert, something went wrong
```

Note: it is useful to add some extension to the editing software you are using to color code the different comments.