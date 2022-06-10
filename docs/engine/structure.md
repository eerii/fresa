# Engine Structure

### Core

This section includes all the most basic structures that are used throughout `fresa`.

In `types` you can find type definitions and aliases, as well as helpful functions for manipulating different types. `config.h` includes all the global configuration variables, that must be initialized before running the application. `f_time.h` and `f_math.h` contain time management and useful operations respectively.

`game.h` is the backbone of the engine, and handles initialization, update and closing. It calls all the other `fresa` functions.

Finally, `log.h` is used for different types of printing to the console.

### Graphics

_This part is being actively refactored._

### Events

### Serialization

### ECS

### GUI

### Audio