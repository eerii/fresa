# [`unit tests`](https://github.com/josekoalas/fresa/blob/main/tools/unit_test.h)

The engine include a simple unit test framework. It works without macros, leveraging lambda functions. Let's start with an example:

```cpp
TestSuite suite("test_name", []{
    "test one"_test = []{ return expect(1 == 1); };   // passes
    "test two"_test = []{ return expect(1 == 2); };   // fails
});
```

Here we are creating a test suite, which consists of a name and a function with a series of tests. A test is defined using the literal `""_test`. The string parameter is the test name for displaying purposes, and this can be assigned to another function that returns an expect call. The function can perform extra calculations, but the return statement must be a boolean expression encapsulated in an expect. If the expression is true, then the test passes, and fails otherwise.

To **run a test** first you need to enable the test system with the preprocessor directive `FRESA_ENABLE_TESTS`. This directive must encapsulate all test code to allow to remove it easily from production builds. Then, the easiest way to configure which suites are run is to change the [engine config](../config.md) parameter `run_tests`. It must be a comma separated list of test suite's names, like so:

```cpp
constexpr inline struct _EngineConfig : EngineConfig {
    constexpr str_view virtual run_tests() const { return "suite_one,suite_two"; };
} engine_config;
```

This code will run the test suites indicated before the engine's startup. If the [log level](log.md) includes `LogLevel::TEST`, then the test results will be print to the console.

Unit tests are provided for all mayor **fresa** features, such as [type definitions](https://github.com/josekoalas/fresa/blob/main/tests/type_tests.cpp), the [math library](https://github.com/josekoalas/fresa/blob/main/tests/math_tests.cpp) and the [job system](https://github.com/josekoalas/fresa/blob/main/tests/job_tests.h). You can check those files to see how suite definitions work in practise. For the sake of organization, all **fresa** internal tests are located on source files in the `tests` folder.