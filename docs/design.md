# engine design

## data oriented design

**fresa :strawberry:** will try to adhere to a _data oriented design_. Object oriented programming is not always suitable to game engines, it can get too messy too soon. I enjoy much more designing engines with data in mind, where it is separated from the functions that use it, providing a clearer and easier to debug code. It is also more efficient in most cases. There are many articles detailing how data oriented design can be better for many reasons (_insert reference_), however, I will stick to it since I have made engines in OOD and DOD and I like data programming much better.

This engine will not be 100% perfectly data oriented, since in some problems it makes more sense to take other approaches. I will do my best to follow its core principles where it feels right to do so.

Just a quick note, I have seen many places use DOD and ECS used interchangeably. Though **fresa :strawberry:** integrates a simple ECS for managing the game entities, and ECS is just an example of data oriented design. It can span all the pieces and frameworks of an engine, it is not wise to reduce it only to an ECS.

## libraries

**fresa**'s approach to libraries is similar to its approach to language features. Libraries will be only added when they provide a sustancial improvement over hand made code, or at least until I can work on a custom version. For example, `vulkan memory allocator` significantly improves memory handling in `vulkan` and it is really well constructed.

When choosing a library, its size will be pretty important. Adding a one file header library will be preferred to a big and complex project. Also, I would like to be able to easily review all the library code and make any necessary modifications (if licensing allows it) for it to be optimized for **fresa :strawberry:**. It will also be nice if they share the same coding style as the engine, for example, with a focus on compile time calculations and with intentful use of new `c++` standards.

All libraries will be linked in the project template **aguacate :avocado:** as submodules, under the folder `deps`, so it is effortless to start a new **fresa :strawberry:** project with them. They will be also listed here with propper attribution and licensing information.