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

If you wish to configure how the engine behaves, you can look into the [configuration file](../reference/config.md) and into the [system manager](../reference/ecs.md#systems).

## hello world

For this example we are going to use a simple system with an [`init()`](../reference/ecs.md) function, which will execute its code when the engine is initialized. We will create a `hello_world.h` file and define this system as so:

```cpp title="hello_world.h"
#pragma once
#include "system.h"

namespace fresa
{
    struct SomeSystem {
        inline static System<SomeSystem> system; //: this line registers the system

        static void init() {
            //: your code goes here
        }
    };
}
```

You also need to include it inside `main.cpp`:

```cpp title="main.cpp"
#include "engine.h"
#include "hello_world.h"

//... other code
```

Now, we can import the [logging](../reference/tools/log.md) library and use it to print a message when the callback is executed:

```cpp title="hello_world.h"
#include "engine.h"
#include "log.h"

//... other code

static void init() {
    log::info("hello world!");
}
```

If you execute the program now, you will see the message printed in the terminal.