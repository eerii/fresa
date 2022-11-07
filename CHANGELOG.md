# changelog :sparkles:

### 0.5 milestone

_the graphics engine_

**todo**

- multiple monitor testing
- write documentation for the new renderer
- documentation for fresa enums
- think of what to do with regards to web builds, probably webgpu
- new job system, look into std::async or the taskflow library (the work stealing queue)
- ci for windows

**vulkan todo**

- subpass
- renderpass
- vertex descriptions
- buffers
- create descriptor resources
- dont render into the swapchain directly (don't create fb for swapchain, instead blit image into it)

---

#### [0.5.6] render graph (_00 nov 22_)

- **changed** - made type names and hashes consteval
- **changed** - switched config to consteval
- **changed** - fixed some comments
- **added** - hot reloading for files
- **added** - simple render graph support

#### [0.5.5] attachments (_17 oct 22_)

- **added** - create vkimage and vkimageview
- **added** - texture data struct and creation
- **added** - create attachments
- **added** - enumeration utilities, adds custom bitwise operators to enum structs
- **added** - bidirectional map
- **changed** - cleanup of api and window implementation logic

#### [0.5.4] pipelines (_17 sept 22_)

- **added** - pipeline layout
- **changed** - moved r_vulkan and r_types contents to r_api and renamed r_vulkan_api to r_api_vulkan
- **changed** - only r_api_* should be able to modify the api object after creation, added new private m_api to allow this
- **added** - pipeline configuration and creation (still needs to add renderpasses and vertex descriptions)

#### [0.5.3] shaders and spirv (_17 sept 22_)

- **added** - spirv code reader
- **added** - resource folder which automatically syncs to the binary folder
- **added** - specific file system for macos
- **added** - shader compilation during build using glslc
- **changed** - moved window definitions to r_window.h and refactored the window code
- **added** - new r_types.h for general graphics import
- **changed** - now there are soft and strong assertions
- **added** - monitor detection and refresh rates
- **changed** - now the game loop sleeps if there is no work to do
- **added** - create shader module
- **added** - integrated spirv cross as a submodule
- **added** - reflect descriptor layout bindings abnd layout creation
- **added** - descriptor pools and descriptor sets

#### [0.5.2] sync objects (_13 sept 22_)

- **added** - created semaphore and fences
- **added** - window and swapchain resizing
- **changed** - switch to enum structs for type correction

#### [0.5.1] commands and memory (_13 sept 22_)

- **added** - vulkan memory allocator library
- **added** - swapchain
- **added** - time format for fmt library
- **removed** - the job system is temporarily removed, since it is not ready and doesn't add anything to the engine
- **removed** - coroutine implementation is also moved to a gist for now
- **added** - command pools and buffers
- **fixed** - string library name error on linux
- **added** - setup ci for macos and linux
- **changed** - engine now has a quit and force quit function
- **added** - window closing callback stops the engine

#### [0.5.0] vulkan initialization (_26 ago 22_)

- **added** - glfw and glad extensions
- **added** - vulkan instance, debug and gpu implementation
- **added** - device queues, surfaces, logical device
- **added** - valid state assertions

---

### 0.4 milestone

_new implementation of fresa_

#### [0.4.8] documentation and final touches (_17 ago 22_)

- **fixed** - strong types can now propperly be formatted
- **added** - reflection format operator
- **added** - documentation for reflection, events and jobs

#### [0.4.7] reflection (_16 ago 22_)

- **added** - new automatic system registration using static members
- **removed** - removed engine callbacks in favor of system initialization
- **removed** - system tests are now more difficult to write due to automatic registration
- **changed** - source location implementation is now moved to source_loc.h
- **added** - runtime assertions
- **added** - ecs and systems documentation
- **added** - struct field counter with binary search and mitigations
- **added** - tie as tuple helper and python generator
- **added** - equality and hash operations for reflectable types
- **added** - tool assisted generation of struct field names

#### [0.4.6] events (_12 ago 22_)

- **added** - system simulation update loop
- **added** - user configurable engine callbacks for initialization and shutdown
- **fixed** - small ecs fixes to make it compile in gcc
- **added** - simple event system

#### [0.4.5] ecs (_10 ago 22_)

- **added** - constexpr type hashing using fnv1a
- **added** - entity id with index and version for ecs
- **changed** - moved log level into a configuration parameter
- **added** - unit test and coroutine documentation
- **added** - ecs sparse allocator
- **added** - ecs scene manager
- **changed** - renamed template repository from aguacate to mermelada (actually makes sense now :3)
- **fixed** - sparse container now works better and preserves entity ids
- **added** - ecs scene views
- **added** - more ecs tests, including multiple sparse memory pages
- **changed** - systems can now optionally contain the stop function
- **changed** - ecs get returns an std::optional instead of a pointer

#### [0.4.4] strong types (_08 jul 22_)

- **added** - strongly typed definitions
- **added** - log documentation
- **changed** - separated fresa_types.h into constexpr_for.h and type_name.h
- **changed** - cleaner and more minimal header imports
- **added** - no discard attributes to appropiate functions

#### [0.4.3] configuration (_03 jul 22_)

- **added** - engine configuration variables using constexpr functions
- **removed** - deprecated command line options in favor of the configuration file for now
- **changed** - now unit tests are defined in the configuration file
- **fixed** - job system can now schedule to both global and local queues
- **added** - math library documentation
- **added** - constexpr factorial, power and smoothstep definitions
- **added** - linear and non linear interpolation
- **added** - three types of configuration: engine, run and debug

#### [0.4.2] numbers and letters (_29 jun 22_)

- **added** - random number generator
- **changed** - better concept names and organization
- **added** - constexpr for
- **added** - vector linear algebra
- **added** - matrix linear algebra
- **changed** - included test guard for production
- **fixed** - constexpression logging with fmt
- **added** - mkbuild documentation system
- **changed** - divided repositories (fresa and fresa-legacy)
- **added** - documentation structure and getting started
- **fixed** - mutex import in atomic queue was missing

#### [0.4.1] job system (_21 jun 22_)

- **added** - string utilities (lowercase, split)
- **changed** - tests are now in standalone files in the tests/ directory and can be run using arguments "-t test1,test2"
- **added** - coroutine implementation (future, promise, awaitables)
- **added** - job system with coroutines, parent-children jobs and multithread support
- **added** - temporary dependency `jthread` while compilers gain supports
- **added** - thread-safe atomic queue

#### [0.4.0] core features (_16 jun 22_)

- **added** - constexpr type name string view
- **added** - `<concepts>` and `<ranges>` check and import
- **added** - simple logging system with different levels and color support
- **added** - unit test framework without macros
- **added** - time clock implementation
- **added** - engine base and fixed update loop
- **added** - system manager