# fresa :strawberry:

A tiny game engine made in C++20.

<p float="left">
  <img src="https://user-images.githubusercontent.com/22449369/145628926-ca734a35-6a0e-4193-872b-4be45b886a48.gif" width="24%" />
  <img src="https://user-images.githubusercontent.com/22449369/145629231-f2f51bd6-330a-4533-9b1a-021ce0859508.gif" width="24%" />
  <img src="https://user-images.githubusercontent.com/22449369/145630097-151555b5-30fc-4fef-b062-72e9581a5731.png" width="24%" />
  <img src="https://user-images.githubusercontent.com/22449369/156197502-0e555a4a-8a71-4920-bb01-b29cefcf4a25.gif" width="24%" />
</p>

## features :sparkles:

**Graphics** 
- Custom renderer with Vulkan, OpenGL and WebGL support
- Instanced rendering
- Multiple shader subpasses
- Automatic GLSL shader compilation for each API and reflection using SPIRV-Cross
- High level API that allows complete customization of the cross platform renderer
- Compute shaders and Indirect drawing (Vulkan only)

**ECS**
- Data oriented Entity Component System
- Scene management
- Custom component controllers for specialized behaviour
- Entity editor GUI

**Reflection**
- C++ compile time type reflection implementation
- Loop through each member of a struct and apply callables
- Automatic serialization (save/load)

**Other**
- Custom compile time state machine implementation
- Input handling
- Logging and debugging tools
- Collision checking
- Time managment and timers
- Event handling

## documentation :book:

A work in progress documentation is available in [josekoalas.github.io/fresa](https://josekoalas.github.io/fresa/).

## building :hammer:

Right now the project is in the **very pre-alpha** state, so use it at your own risk.

**Examples:**
- :avocado: [aguacate](https://github.com/josekoalas/aguacate): Full **fresa** template, with detailed instructions on how to build
- :sun_behind_large_cloud: [raymarching](https://github.com/josekoalas/maracuya/tree/main/raymarching): Quick and simple raymarching example

You can check  for an example on how one could set up a **fresa** project. Instructions on how to build it are there.

**Dependencies**
- SDL2 (cross-platform window and input)
- Vulkan/OpenGL (renderer of choice)
- SPIR-V Cross (shader reflection and compilation)
- VulkanMemoryAllocator (only Vulkan renderer, memory management)
- imGUI (for a debug graphical interface)
- stb_image (loading images)
- glm (glsl linear algebra)

**Options**
- `USE_VULKAN` or `USE_OPENGL`: Enables the desired renderer
- `LOG_LEVEL = 1...5`: Selects log verbosity, 1 being only errors and 5 debug
- `DISABLE_GUI`: Disables the compilation of imGUI and all the GUI code
- `PROJECT_DIR`: For debugging editor tools, the root of your project

## code example :books:

**main.cpp**

```cpp
#include "game.h"
#include "scene.h"

using namespace Fresa;

int main(int argc, const char * argv[]) {
    //: Initialize game
    bool running = Game::init();
    
    //: Load scene
    SceneID scene = registerScene();
    active_scene = scene;
    
    //: Update loop
    while (running) {
        running = Game::update();
    }
    
    //: Cleanup
    Game::stop();
    
    return EXIT_SUCCESS;
}
```

**other files**

To access the ECS capabilities, a `component_list.h` file may be created that `#includes` all the component headers you create, as well as a variant for the component type. Follow the instructions on [`ecs.h`](https://github.com/josekoalas/fresa/blob/main/ecs/ecs.h) or the example in [aguacate](https://github.com/josekoalas/aguacate) to create one.

You can also create a `vertex_list.h` to add new vertex type definitions to use with glsl.

## license :pencil:

This project is licensed under GNU GPLv3. I made **fresa** :strawberry: because I wanted to learn, and I did, so many things. But this learning was possible only because there are thousands of awesome people that publish their projects for others to see. That is why I am so happy, after all this time, I made something I consider it is worth sharing. It is not perfect, and it has room to grow, but it is something.

This is the reason I chose this license. It allows anyone to see, use, modify and learn from this code. There is one condition though, that all derivative distributed projects must also be **shared** with the same license, as open source code. That way other people might learn from what we make in the future, and they might end up creating something much better.

Here is a brief explanation of the license:

```markdown
1. Anyone can copy, modify and distribute this software.
2. You have to include the license and copyright notice with each and every distribution.
3. You can use this software privately and for commercial purposes.
4. If you modify it, you have to indicate changes made to the code.
5. Any modifications of this code base MUST be distributed with the same license, GPLv3.
6. This software is provided without warranty.
7. The software author or license can not be held liable for any damages inflicted by the software.
```

For reference, please read the full license [here](https://github.com/josekoalas/fresa/blob/main/LICENSE.md).
