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

Namespaces will always try to be only one word. Interfaces will start with a capital "I" and concepts will start with "C". These conventions might be broken when working with elements defined in external libraries, such as `vulkan`.

We will also denote the engine name in fully lowercase letters and accompanied by the strawberry emoji, **fresa :strawberry:**. All titles of documentation sections will be similarly written in lowercase.

## comments and documentation

```cpp
//: regular comments
//* headers
//- todos
//? implementation questions
//! alert, something went wrong
```

Subsecuent lines of comments (that are not documentation) can be tabbed 2 times to improve readability. It can also be useful to add some extension to the editing software you are using to color code the different comments.