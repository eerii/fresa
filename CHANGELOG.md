### changelog :sparkles:

### 0.3 - duck :duck:

---

#### 0.3.4 - 

**added**
- compile time type name generator

**changed**
- removed macros from state machine name generation and changed it to compile time names

**up next**
- state machine refactoring with new events!

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
