# [`config`](https://github.com/josekoalas/fresa/blob/main/core/fresa_config.h)

The configuration system defined in `fresa_config.h` allows for configurable constexpr variables. The struct `Config` contains constexpr virtual methods (a new `c++20` feature), allowing to create an interface that can be extended but retaining the compile time definitions. It is a bit more verbose than other approaches, but being able to extend and override constant expressions is very powerful. Look at this example, here we have the original config structure:

```cpp
struct fresa::Config {
    constexpr str_view virtual name() const { return "fresa"; };
};
```

Which can be overwritten as:

```cpp
struct GameConfig : fresa::Config {
    constexpr str_view name() const override { return "aguacate"; }
};
```

## configuration file

**fresa** allows you to create a configuration file to overwrite the default options. To do so, create a header file and include it in the project. By default the name of the configuration file is `config.h`, but if you wish to use a different name you can use the `FRESA_CONFIG_FILE` preprocessor directive as `FRESA_CONFIG_FILE = file_name.h` (only write the file name, excluding its path). An example configuration file is provided with the [template](https://github.com/josekoalas/aguacate) repository, similar to this:

```cpp title="config.h"
#pragma once
#include "fresa_config.h"

namespace fresa
{
    constexpr inline struct GameConfig : Config {
        //: change the project name to "my project"
        constexpr str_view name() const override { return "my project"; }
    } config;
}
```

This configuration file:

- Needs to import `fresa_config.h`.
- Creates a struct that extends `fresa::Config`.
- Overrides some of the constexpr methods with new configurations.
- **Creates an inline variable named `config`.**

If you look at the [`fresa_config.h`](https://github.com/josekoalas/fresa/blob/main/core/fresa_config.h) code, you will see that the configuration file, if present, is imported back into that header (using guards to avoid compilation loops). That means that the `config` variable will be available engine-wide, allowing every system to access this overwritten configuration.

## using configuration

All you need to access the configuration variables is to include "fresa_config.h" and call any option from `config`.

```cpp
#include "fresa_config.h"

void using_configuration() {
    log::info("{}", config.name());
}
```

## configuration options

| option | type | default value |
|---|---|---|
| `name` | `str_view` | `"fresa"` |
| `version` | `std::array<ui8, 3>` | `{0, 4, x}` |
| `run_tests` | `str_view` | `""` |