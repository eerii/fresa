# a simple program

If you are using the [template](building.md#using-a-template) then you already have a minimum `main.cpp` file that should look something like this:

```cpp
#include "engine.h"

int main (int argv, char** args) {
    fresa::run(argv, args);
    return 0;
}
```

All you need to run **fresa** is to import the engine header and call `fresa::run`. Optionally you can pass the program arguments directly to use command line options.

_This page will be updated when components and configuration are added to the engine to explain how to create your own functionality._