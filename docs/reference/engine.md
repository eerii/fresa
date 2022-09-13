# [`engine`](https://github.com/josekoalas/fresa/blob/main/core/engine.h)

This is the main class of **fresa**. Handles initialization, update and cleaning, calling all required systems.

## `run`

```cpp
void fresa::run()
```

This is the main entrypoint of the engine. First it perform the tests defined in the configuration file (if enabled with `FRESA_ENABLE_TESTS`). Then calls [`init`](#init), which handles system initialization.

Finally it calls the main `update` loop of the application. The simulation update is decoupled from the frame time, instead being increased in discrete steps of `dt`. This allows the engine to run independent of frame rate. The implementation is very similar to the one described in the [fix your timestep](https://gafferongames.com/post/fix_your_timestep) article.

## `quit` and `force_quit`

```cpp
void fresa::quit()
void fresa::force_quit()
```

Can be called when the engine needs to exit early. This will call [`stop`](#stop) and then exit the application. There are two variants: the regular `quit` first completes the current frame of the update loop, and then exists, while `force_quit` runs `stop` and exits immediately.