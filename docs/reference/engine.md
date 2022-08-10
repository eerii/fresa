# [`engine`](https://github.com/josekoalas/fresa/blob/main/core/engine.h)

This is the main class of **fresa**. Handles initialization, update and cleaning, calling all required systems.

## `run`

```cpp
void fresa::run()
```

This is the main entrypoint of the engine. First it perform the tests defined in the configuration file (if enabled with `FRESA_ENABLE_TESTS`). Then calls [`init`](#init) before running the update loop. Finally it stops all systems and returns control to the caller function.

## `init`

```cpp
void fresa::detail::init()
```

Called from [`run`](#run) handles system initialization and all work needed before the main update loop.

## `update`

```cpp
bool fresa::detail::update()
```

Main update loop of the application. The simulation update is decoupled from the frame time, instead being increased in discrete steps of `dt`. This allows the engine to run independent of frame rate. The implementation is very similar to the one described in the [fix your timestep](https://gafferongames.com/post/fix_your_timestep) article.

## `stop`

```cpp
void fresa::detail::stop()
```

Called from [`run`](#run) when execution ends. Clear systems in LIFO order (using an `std::stack`).

## callbacks

The engine also includes user configurable callbacks that are executed to modify the engine behavior. For example, the `on_init()` callback can be used to add systems. This callbacks can be configured similar to how the `EngineConfig` struct in [config.h](config.md) works.

In order to customize the callbacks you may create a header file that includes `engine.h` and defines an `EngineCallback` derived struct. The name of this struct is defined by the `FRESA_ENGINE_FILE` preprocessor directive, which defaults to `game.h`, and it can be configured by `FRESA_ENGINE_FILE = file_name.h` (only write the file name, excluding its path). An example engine file is provided with the [template](https://github.com/josekoalas/mermelada) repository, similar to this:

```cpp title="game.h"
#pragma once

#include "engine.h"
#include "log.h"

namespace fresa
{
    constexpr inline struct _EngineCallback : EngineCallback {
        void on_init() const override { 
            log::info("hey");
        }

        void on_stop() const override {}
    } engine_callback;
}
```

The available callbacks are:

- `on_init()`: called when the engine is initialized, at the end of the `init()` function.
- `on_stop()`: called when the engine is stopped, at the start of the `stop()` function.