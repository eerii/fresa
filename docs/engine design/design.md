# engine design

## data oriented design

**fresa :strawberry:** will try to adhere to a _data oriented design_. Object oriented programming is not always suitable to game engines, it can get too messy too soon. I enjoy designing engines with data in mind, with it being separated from the functions that use it, providing cleaner and easier to debug code. It can also be more performant in a variety of situations. There are many articles detailing how data oriented design can be better for many reasons (_insert reference_), however, I will stick to it since I have made engines in OOD and DOD and I like data programming much better. Please note that this engine will not always be 100% perfectly data oriented, since in some problems it makes more sense to take other approaches.

Just a quick note, I have seen many places use DOD and ECS used interchangeably. Though **fresa :strawberry:** integrates a simple ECS for managing the game entities, an ECS is just an example of what data oriented design can do. It can span all the pieces and frameworks of an engine, it is not wise to reduce it only to an ECS.

## libraries

**fresa**'s approach to libraries is similar to its approach to language features. Libraries will be only added when they provide a sustancial improvement over hand made code, or at least until I can work on a custom version. For example, `vulkan memory allocator` significantly improves memory handling in `vulkan` and it is really well constructed.

When choosing a library, its code size will be pretty important. Adding a small one file header library will be preferred to a big and complex project. I will try to only add libraries (or parts of them) which _only_ focus on the problem I'm trying to solve by using them. Also, I would like to be able to easily review all the library code. It will also be nice if they share the same coding style as the engine, for example, with a focus on compile time calculations and with intentful use of new `c++` standards.

All libraries will be linked in the project template **aguacate :avocado:** as submodules, under the folder `deps`, so it is effortless to start a new **fresa :strawberry:** project with them. They will be also listed here with propper attribution and licensing information.

- **fmt**: This library could easily be part of the std library. It is easy to follow, has a really nice syntax and vastly simplifies printing and formatting strings. Besides, it is really performant. I use it as a basis to the logging system (`log.h`), since it also allows to use formatting and colors. It may not be strictly required for our implementation, but a very welcomed quality of life.
- **range-v3**: This is a temporary fix since compiler support for the `<ranges>` of `c++20` is not finalised (specially in clang, the compiler I am using). The code will try to import `<ranges>`, and if not fall back to `<range-v3>`, which should be pretty much compatible since the standard is based off it. Once compiler support is more widespread it will be removed.
- **jthread**: Another temporary fix since `std::jthread` is not yet supported. Using the library that the standard implementation is based, so there should be no noticeable differences.