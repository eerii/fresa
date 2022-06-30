# [`time`](https://github.com/josekoalas/fresa/blob/main/core/fresa_time.h)

Small time management utility based on `std::chrono`. Defines helpful functions to manage the flow of time. It also imports the chrono literals for time, for example, `1ms`.

## `clock`

```cpp
fresa::clock = std::chrono::steady_clock;
```

## `time`

```cpp
void fresa::time()
```

Returns the current point in time.
