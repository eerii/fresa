# fresa :strawberry:

A tiny game engine made in C++20.

<p float="left">
  <img src="https://user-images.githubusercontent.com/22449369/145628926-ca734a35-6a0e-4193-872b-4be45b886a48.gif" width="30%" />
  <img src="https://user-images.githubusercontent.com/22449369/145629231-f2f51bd6-330a-4533-9b1a-021ce0859508.gif" width="30%" />
  <img src="https://user-images.githubusercontent.com/22449369/145630097-151555b5-30fc-4fef-b062-72e9581a5731.png" width="30%" />
</p>

## features :sparkles:

**Graphics** 
- Custom renderer with Vulkan, OpenGL and WebGL support.
- Multiple subpasses
- Automatic shader compilation for each API and reflection using SPIRV-Cross
- Supports the creating of vertex buffers and textures at the start of the program for better performance (they can also be created dynamically)

**ECS**
- Data oriented Entity Component System
- Scenes
- Custom component controllers for specialized behaviour

**Reflection**
- C++ compile time type reflection implementation
- Loop through each member of a struct and apply callables
- _Automatic serialization (save/load) WIP_

**Other**
- Custom compile time **state machine** implementation
- Input handling
- Logging and debugging tools
- Collision checking
- Time managment and timers
- Event handling
- _GUI (legacy, new version upcoming)_

## building :hammer:

**Platforms**

Right now the project is in the **very pre-alpha** state, and I haven't set up build instructions for all platforms yet. Only macOS is functional and web is semi-functional. Please stay tuned for updated building instructions, or message me and I'll send you what I have now. I intend to support all mayor OS (Linux, macOS and Windows), as well as WebGL and mobile devices, but it is not there just yet.

**Dependencies**
- SDL2 (cross-platform window and input)
- Vulkan/OpenGL (renderer of choice)
- imGUI (for a debug graphical interface)
- SPIR-V Cross (shader reflection and compilation)
- stb_image (loading images)
- VulkanMemoryAllocator (only Vulkan renderer, memory management)

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

ECS and other descriptions are defined in 4 separate files that must be created:
- `component_list.h`
- `system_list.h`
- `controller_list.h`
- `state_machines_list.h`

I am working on a repository with a minimum working example, but it is not commented propperly just yet.

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
