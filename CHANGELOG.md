### changelog :sparkles:

### 0.3 - duck :duck:

---

#### 0.3.13 - TITLE - _00 00 2022_

**added**
- propper 3d camera controller
- camera gui
- debug attachments
- multisampling (vulkan)
- multiple buffer support

**changed**
- rendering api fixes in vulkan

**fixed**
- mouse input was not working

---

#### 0.3.12 - indirect drawing - _23 03 2022_

**added**
- vulkan indirect drawing
- improved uniform binding and global uniforms
- initial support for compute shaders in vulkan
- storage buffer support
- automatic reflection for storage and uniform buffers using spirv-cross
- compute shader support!

**changed**
- improved the instanced queue
- improved uniform set updating with support for more attributes

---

#### 0.3.11 - instanced rendering - _15 03 2022_

**added**
- instanced rendering!
- gpu timestamp gui (not working on moltenvk)
- loop audio
- callback and event timers

**changed**
- multiple vertex types in attribute description (to allow instance rendering vertices)
- complete rework of the rendering queue and rendering data

**fixed**
- rendering description not loading on web due to path issue

---

#### 0.3.10 - it's your favourite song - _13 03 2022_

**added**
- preliminary audio support
- indexed obj loading

**chaged**
- variable size index buffer

**fixed**
- cmake include mess

---

#### 0.3.9 - bip bop bup - _12 03 2022_

**added**
- gui menu information
- performance gui with detailed timings
- font support for imgui

**changed**
- renamed callTime to TIME to make it clear it is a debug element

---

#### 0.3.8 - say my name, say my name - _11 03 2022_

**added**
- entity serialization, loading from file
- log can handle std::vectors
- new experimental reflection
- reflection can apply functions to members by name
- serialization now uses the reflection api
- scene serialization with support for entity templates
- system init function

---

#### 0.3.7 - housekeeping - _07 03 2022_

**added**
- include custom vertex types creating a vertex_list.h file

**changed**
- shader names used as id, automatic shader detection
- improved renderer description
- component_list.h is now optional (still required to use ecs features)
- config values are now defined outside of fresa, to be easily modifiable

**fixed**
- vulkan memory issue that was not detected before due to xcode's address sanitizer :c

---

#### 0.3.6 - looking sharp - _11 02 2022_

**added**
- imgui support
- orthographic perspective
- new gui menu
- multiple sized attachments for scaled rendering
- bidirectional maps (new and improved)
- opengl multiple image support
- renderer description with parser
- scaled rendering

**changed**
- refactored update lists
- viewport refactoring
- moved attachment registering to the API
- improved logging
- improved modules and imports

**fixed**
- hopefully fixed sdl_keycodes not being recognised on linux and web
- culling was reversed on vulkan

#### 0.3.5 - make make make and make - _24 12 2021_

**added**
- performance timings to physics and rendering
- tried to add vulkan timestamps 
- camera struct with projection and view matrices

**changed**
- disable validation layers on release
- filesystem path helper for adding base path (in emscripten it is different)

**fixed**
- fixed vulkan render pass dependency flag validation error
- temporary fix for msvc compilers, disabled logging since there was a compilation error
- unsupported validations layers no longer crash the program, just issue a warning
- fixed shader compiling for web
- fixed an error with opengl input attachments, that seemed to work but it was a coincidence

#### 0.3.4 - all about that state c: - _18 12 2021_

**added**
- compile time type name generator
- component list documentation
- state machine events can now be linked with new event system
- render system list caller inside r_graphics.h

**changed**
- removed macros from state machine name generation and changed it to compile time names
- changed state machine `is()` function to remove accidental RTTI

**fixed**
- fixed vulkan only working when the xcode address sanitizer was on (errors with use of memory after return)
- fixed emscripten window and float precision

#### 0.3.3 - hey there! this is an event - _14 12 2021_

**added**
- publish events
- observers and permanent callbacks
- new variant helper functions
- improved component id generation based on variant index

**changed**
- improved event handling
- reworked input
- refactored type headers

#### 0.3.2 - component nightmare - _13 12 2021_

**added**
- heterogeneous poly container

**changed**
- improved `getID<Component>` and added special checks
- now system definition is independent of the engine, they can be registered in the update lists by template instantiation

#### 0.3.1 - out to the world - _11 12 2012_

**added**
- readme and new changelog (kinda meta)

**changed**
- improved documentation
- refactored config struct

#### 0.3.0 - the rendering update - _09 12 2021_

**added**
- brand new rendering api with complete vulkan support
- shader reflection with spirv-cross
- multiple subpasses and attachments
- many more changes to the engine

---

_previously fresa was tied to another of my projects, lume, so the changelog was shared between them. this changelog therefore starts at version 0.3, when fresa was made completely standalone._
