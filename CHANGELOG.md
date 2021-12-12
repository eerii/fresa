### changelog :sparkles:

### 0.3 - duck :duck:

---

#### 0.3.2 - component nightmare - _12 12 2021_

**added**
- heterogeneous poly container
- ~~reflection reference type eraser interface~~
- ~~automatic component type list~~ (this posed more problems than it was worth, the variant version works ok for our needs)

**changed**
- improved `getID<Component>` and added special checks
- now `component_list.h` and `system_list.h` are merged into `ecs_description.h`, this is likely temporary, since it is being refactored still

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
