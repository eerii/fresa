# language features

**fresa :strawberry:** uses the `c++20` standard and some of its most recent features. The `std` library contains many useful structures and design patterns that were implemented in the past few years. I will carefully pick which features I use and document its usage in this file.

This is an opinionated engine, features will be added with a clear intent, keeping clutter to a feasible minimum. These are all my opinions and they are not right nor wrong, just how I like to code now.

Keeping performance in mind, a key feature of any game engine on a tight per-frame time budget, compile time calculations and templated code will be a fundamental in the design of this engine. There will also be an active attempt to try to avoid macros (except for conditional compilation) and replace them with safer and more readable alternatives where possible. Macros will always be capitalized and clearly indicated.

### strings

- std::string (aliased as str)
- std::string_view (aliased as str_view)

### containers

- std::vector
- ranges (`c++20`)

### metaprograming

- templates
- lambdas
- concepts (`c++20`)

