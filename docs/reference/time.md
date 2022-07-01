# [`time`](https://github.com/josekoalas/fresa/blob/main/core/fresa_time.h)

Small time management utility based on `std::chrono`. Defines helpful functions to manage the flow of time. It also imports the chrono literals for time, for example, `1ms`.

The clock implementation is `std::chrono::steady_clock`, aliased as `fresa::clock`. The main function of the library is `fresa::time()`, which returns the current point in time.