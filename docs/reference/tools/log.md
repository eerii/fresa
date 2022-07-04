# [`log`](https://github.com/josekoalas/fresa/blob/main/tools/log.h)

**fresa** has a simple logging system using the [`fmt`](https://github.com/fmtlib/fmt) library. The function `fresa::detail::log` is a passthrough to `fmt::print` with extra parameters for special formatting.

```cpp
fresa::detail::log<"NAME", LOG_LEVEL, fmt::color>("{}", ...);
> [NAME]: ...
```

The `name` template parameter is the type of log message, `LOG_LEVEL` is used to enable or disable log types, and a color can be passed to make the output more readable. The first function parameter must be an [`fmt::format_string`](https://fmt.dev/latest/syntax.html). Simplifying, this means that it will behave as a regular string but everywhere there is a "{}" it will be replaced with the next argument. Custom printing formats can be passed, for example, as "{:.2f}". Leveraging the new features introduced in `fmt` with `c++20`, the passthrough can now use constant expressions to format at compile time.

## log levels

The logging level can be specified using the `LOG_LEVEL` preprocessor directive. Each log level is defined in a bitwise enum, so they can be granularly combined.

| log level | bit flag |
|---|---|
| `LOG_ERROR` | 1 << 0 |
| `LOG_WARNING` | 1 << 1 |
| `LOG_INFO` | 1 << 2 |
| `LOG_GRAPHICS` | 1 << 3 |
| `LOG_TEST` | 1 << 4 |
| `LOG_DEBUG` | 1 << 5 |
| `LOG_JOBS` | 1 << 6 |

For example, `LOG_LEVEL = 0b0010011` enables logging for errors, warnings and unit test results.

## predefined log functions

There are a number of predetermined log functions as a convenience:

| function | name | log level | color |
|---|---|---|---|
| `log::error` | "ERROR" | `LOG_ERROR` | red |
| `log::warn` | "WARN" | `LOG_WARN` | gold |
| `log::info` | "INFO" | `LOG_INFO` | cornflower_blue |
| `log::graphics` | "GRAPHICS" | `LOG_GRAPHICS` | dark_turquoise |
| `log::debug` | "DEBUG" | `LOG_DEBUG` | light_sky_blue |