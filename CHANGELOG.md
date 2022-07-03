# changelog :sparkles:

### 0.4 milestone

_new implementation of fresa_

#### todo
- engine config vs game config (constexpr vs runtime and modifiable)
- ecs
- events
- file management and parser
- reflection
- continue with documentation

#### [0.4.3] name (_2 jul 22_)

- **added** - engine configuration variables using constexpr functions
- **removed** - deprecated command line options in favor of the configuration file for now
- **changed** - now unit tests are defined in the configuration file
- **fixed** - job system can now schedule to both global and local queues
- **added** - math library documentation
- **added** - constexpr factorial, power and smoothstep definitions
- **added** - linear and non linear interpolation

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