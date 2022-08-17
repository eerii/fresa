# [`config`](https://github.com/josekoalas/fresa/blob/main/core/fresa_config.h)

The configuration system defined in `fresa_config.h` includes three types of configuration variables:

- `EngineConfig`: Compile time variables that change the way the engine work. These are for things like enabling different parts of the engine or values that don't change.
- `RunConfig`: Run time modifiable values, for example, the settings menus or options that change with the player's input.
- `DebugConfig`: Also run time values, but are disabled in production builds. All usage of this must be guarded using the `FRESA_DEBUG` directive. Useful for debug toggles during development.

## engine config

The struct `EngineConfig` contains constexpr virtual methods (a new `c++20` feature), allowing to create an interface that can be extended but retaining the compile time definitions. It is a bit more verbose than other approaches, but being able to extend and override constant expressions is very powerful. Look at this example, here we have the original config structure:

```cpp
struct fresa::EngineConfig {
    constexpr str_view virtual name() const { return "fresa"; };
};
```

Which can be overwritten as:

```cpp
struct _EngineConfig : fresa::EngineConfig {
    //: change the project name to "mermelada"
    constexpr str_view name() const override { return "mermelada"; }
};
```

## configuration file

**fresa** allows you to create an optional configuration file to overwrite the default options. To do so, create a header file and include it in the project. By default the name of the configuration file is `config.h`, but if you wish to use a different name you can use the `FRESA_CONFIG_FILE` preprocessor directive as `FRESA_CONFIG_FILE = file_name.h` (only write the file name, excluding its path). An example configuration file is provided with the [template](https://github.com/josekoalas/mermelada) repository, similar to this:

```cpp title="config.h"
#pragma once
#include "fresa_config.h"

namespace fresa
{
    constexpr inline struct _EngineConfig : EngineConfig {
        constexpr str_view name() const override { return "my project"; }
    } engine_config;

    inline RunConfig config{
        .something = "initial value"
    };

    #ifdef FRESA_DEBUG
    inline struct DebugConfig debug_config{};
    #endif
}
```

If you chose to create a configuration file, it needs to:

- Import `fresa_config.h`.
- _(optional)_ Create a struct that extends `fresa::EngineConfig` to override some of the constexpr methods with new configurations.
- _(optional)_ Create structs that extend `fresa::RunConfig` and `fresa::DebugConfig`.
- **Create inline variables named `engine_config`, `config` and `debug_config`.** Even if you don't modify the default options of some of these structs, you need to define all three variables _(only if you create a configuration file)_. These variables must be either of the original struct's types or a derived type. `engine_config` also needs to be constexpr.

If you look at [`fresa_config.h`](https://github.com/josekoalas/fresa/blob/main/core/fresa_config.h), you will see that the configuration file, if present, is imported back into that header (using guards to avoid compilation loops). That means that the config variables will be available engine-wide, allowing every system to access this overwritten configuration.

## using configuration

All you need to access the configuration variables is to include "fresa_config.h" and call any option from `engine_config`, `run_config` or `debug_config`. Note that the values in engine config are functions, so they must be called.

```cpp
#include "fresa_config.h"

void using_configuration() {
    log::info("{}", engine_config.name());
    if constexpr (engine_config.version().at(0) == 0) // engine config can be used in compile time expressions
        log::info("alpha");

    run_config.player_name = "mango"; // run and debug config are modifyable

    #ifdef FRESA_DEBUG
    if (debug_config.show_debug_artifacts) // debug config must be guarded
        render_debug();
    #endif
}
```

## configuration options

**engine**

| option | type | default value |
|---|---|---|
| `name` | `str_view` | `"fresa"` |
| `version` | `std::array<ui8, 3>` | `{0, 4, x}` |
| `run_tests` | `str_view` | `""` |
| `enable_assertions` | `bool` | `false` |
| `enable_validation_layers` | `bool` | `false` |
| `log_level` | `ui32` | `0b0000111` |
| `ecs_page_size` | `ui32` | `256` |

**run**

| option | type | default value |
|---|---|---|
|  |  |  |

**debug**

| option | type | default value |
|---|---|---|
|  |  |  |