# [`assertions`](https://github.com/josekoalas/fresa/blob/main/tools/fresa_assert.h)

To check if code is running propperly, **fresa** includes an assertion system. For code that can be checked at compile time, the standard `static_assert()` is recommended. However, for runtime code, it includes `fresa_assert()`. It is used like so:

```cpp
void something(int i) {
    fresa_assert(i > 0, "i must be greater than 0");
    // ...
}
```

If the assertion fails, the program will abort and print a message along with the file and line of the error. If it passes, nothing will happen. Runtime assertions have a small performance penalty since they have to check if the condition is met at runtime, but they are useful debugging tools. Assertions are disabled by default, and should be only used for debugging, not production code. The flag to enable them is inside the [`engine configuration`](../config.md), called `enable_assertions()`.