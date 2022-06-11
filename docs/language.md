# language features

**fresa :strawberry:** uses the `c++20` standard and some of its most recent features. The `std` library contains many useful structures and design patterns that were implemented in the past few years. My goal is to try to implement everything in this engine as elegantly as possible, while learning the new specifications. However, I agree with some implementations more than others, and I also care about code readability. This is why I will be carefully picking which features I use and documenting its usage in this file.

This engine will be opinionated, but I will try to add features to this list with a clear intent, keeping clutter to a feasible minimum. These are all my opinions and they are not right nor wrong, just how I like to code now.

Keeping performance in mind, a key feature of any game engine on a tight per-frame time budget, compile time calculations and templated code will be a fundamental in the design of this engine. There will also be an active attempt to try to avoid macros and replace them with safer and more readable alternatives where possible. Macros will always be capitalized and clearly indicated.

Below is a list of all language features used. Please keep in mind that this list will only include main features, such as `std::vector` or `ranges`, and it will list them under the more relevant standard release. Small changes to existing features will not be included. This list may not be completely exhaustive, and it is not intended to be used for determining compiler compatibility, but instead to describe code formatting and language.

## features

### c++20

### c++17

### c++14

### c++11

### pre c++11