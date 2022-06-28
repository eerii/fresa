# changelog :sparkles:

### 0.4 milestone

_general description_

#### [0.4.2] name (_23 jun 22_)

- **added** - random number generator
- **changed** - better concept names and organization
- **added** - constexpr for
- **added** - vector linear algebra
- **added** - matrix linear algebra
- **changed** - included test guard for production
- **todo** - global/local job queues

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