# a simple program

If you are using the [template](building.md#using-a-template) then you already have a minimum `main.cpp` file that should look something like this:

```cpp
#include "engine.h"

int main (int argv, char** args) {
    fresa::run();
    return 0;
}
```

All you need to run **fresa** is to import the engine header and call `fresa::run()`.

If you wish to configure how the engine behaves, you can look into the [configuration file](../reference/config.md) and how the [engine callbacks](../reference/engine.md#callbacks) work.

## hello world

For this example we are going to use the [`on_init()`](../reference/engine.md#callbacks) engine callback, which will execute its code when the engine is initialized. We will create a `game.h` file and define this funcion as so:

```cpp title="game.h"
#pragma once
#include "engine.h"

namespace fresa
{
    constexpr inline struct _EngineCallback : EngineCallback {
        void on_init() const override {
            // here goes your code
        }
    } engine_callback;
}
```

Now, we can import the [logging](../reference/tools/log.md) library and use it to print a message when the callback is executed:

```cpp title="game.h"
#include "engine.h"
#include "log.h"

//... other code

void on_init() const override {
    log::info("hello world!");
}
```

If you execute the program now, you will see the message printed in the terminal.