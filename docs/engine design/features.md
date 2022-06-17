# language features

**fresa :strawberry:** uses the `c++20` standard and some of its most recent features. The `std` library contains many useful structures and design patterns that were implemented in the past few years. I will carefully pick which features I use and document its usage in this file.

This is an opinionated engine, features will be added with a clear intent, keeping clutter to a feasible minimum. These are all my opinions and they are not right nor wrong, just how I like to code now.

Keeping performance in mind, a key feature of any game engine on a tight per-frame time budget, compile time calculations and templated code will be a fundamental in the design of this engine. There will also be an active attempt to try to avoid macros (except for conditional compilation) and replace them with safer and more readable alternatives where possible. Macros will always be capitalized and clearly indicated.

### strings

- std::string (aliased as str)
- std::string_view (aliased as str_view)

### containers

**common:**

- std::array as the main container, use wherever possible
- std::vector only for containers that must be variable, preallocation is recommended
- std::unordered_map for key-value pairs
- std::map only when ordered access is required
- ranges (`c++20`)

**specific:**

- std::deque for lists where a lot of elements are added to or removed at the back
- std::set for unique identifier lists, useful mathematical properties
- std::stack for LIFO containers, for example, system initialization/destruction
- std::queue and std::priority_queue for other ordered containers

### metaprograming

- templates
- lambdas
- concepts (`c++20`)

### compiler

- feature testing (`c++20`)
- std::source_location (`c++20`) for unit testing

### time

- std::chrono for system independent timers
- coroutines (`c++20`), using experimental branch for clang compilers