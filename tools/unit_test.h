//* unit_test
//      basic unit testing framework with no macros
//      key idea from kris jusiak (https://www.youtube.com/watch?v=-qAXShy1xiE)

//: example
//      TestSuite suite("Some tests", []{
//          "Test one"_test = []{ return expect(1 == 1); };      // passes
//          "Test two"_test = []{ return expect(1 == 2); };      // fails
//      });
//      ...
//      test_runner.run();
#pragma once

#include "std_types.h"
#include "log.h"
#ifdef __cpp_lib_source_location
    #include <source_location>
#endif

namespace fresa
{
    namespace concepts
    {
        //* printable concept
        //      can be printed to the console with a << operator
        template <typename T>
        concept TPrintable = requires (std::ostream &os, T t) {
            os << t;
        };

        //* test concept
        //      needs to have a printable name and to be assignable to an expression
        template <typename T, auto expression = []{}>
        concept TTest = requires(T t) {
            { t.name } -> TPrintable;
            t = expression;
        };

        //* expression concept
        //      can be evaluated to a boolean
        template <typename T>
        concept TExpression = std::convertible_to<T, bool>;
    }

    namespace detail
    {
        //* source location
        //      provides the name of the file and the line number of the function calling it
        //      standard in c++20, but compiler support is lacking, so an alternative is also provided
        #ifdef __cpp_lib_source_location
            using source_location = std::source_location;
        #else
            struct source_location
            {
                static constexpr auto current(
                    #if (__has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE))
                        const char *file = __builtin_FILE(),
                        int line = __builtin_LINE()
                    #else
                        const char *file = "no file",
                        int line = 0
                    #endif
                ) noexcept {
                    return source_location{file, line};
                }

                constexpr auto file_name() const noexcept { return file_; }
                constexpr auto line() const noexcept { return line_; }

                const char* file_;
                int line_;
            };
        #endif

        //* test objects
        //      useful structures to store data for the test system
        namespace test_objects
        {
            //: suite
            struct Suite {
                void (*run)();
                str_view name;
            };

            //: expect result
            struct Expect {
                detail::source_location location;
                bool passed;
            };
        }

        //* test runner
        //      bundles all the test suites and runs them on command
        //      improvement: add tags and filtering to have more granular control over running tests
        struct TestRunner {
            std::vector<test_objects::Suite> suites{};

            //: add suite
            auto add(test_objects::Suite suite) {
                suites.push_back(suite);
            }

            //: run selected suites
            void run (const std::vector<test_objects::Suite> &suites) {
                for (auto suite : suites) {
                    detail::log<"TEST", LOG_TEST | LOG_DEBUG, fmt::color::slate_gray>("Running suite '{}'", suite.name);
                    suite.run();
                }
            }
            
            //: run all
            void run() {
                run(suites);
                suites.clear();
            }

            //: run only some tests
            void run(std::vector<str_view> tests) {
                auto run_suites = suites | rv::filter([&](auto suite) {
                    return ranges::contains(tests, suite.name);
                }) | ranges::to_vector;

                suites = suites | rv::set_difference(run_suites, [](auto a, auto b){ return a.name < b.name; }) | ranges::to_vector;

                run(run_suites);
            }
        };
    }

    //* test runner handle
    inline detail::TestRunner test_runner;

    //* test
    //      single assertment on the form of a lambda with an expect statement inside
    //      if it is standalone, it will run in place
    //      otherwise it will be called from the suite it belongs from the test_runner
    struct Test {
        str_view name;

        void operator=(std::invocable auto test) {
            auto t = test();
            if (t.passed) {
                detail::log<"TEST PASSED", LOG_TEST | LOG_DEBUG, fmt::color::lime_green>("{}", name);
            } else {
                str_view file_name = t.location.file_name();
                file_name = file_name.substr(file_name.find_last_of("/") + 1);
                detail::log<"TEST FAILED", LOG_TEST | LOG_ERROR, fmt::color::red>("{} ({}:{})", name, file_name, t.location.line());
            }
        }
    };

    //* test literal operator
    constexpr auto operator""_test(const char* name, std::size_t size) {
        return Test{.name = str_view{name, size}};
    }

    //* expect
    //      evaluates an expression from a test and returns the result as well as the code location
    constexpr auto expect(concepts::TExpression auto expression, const detail::source_location &location = detail::source_location::current()) {
        return detail::test_objects::Expect{.location = location, .passed = expression};
    }

    //* test suite
    //      a collection of tests
    struct TestSuite {
        str_view name;

        TestSuite (str_view name, std::invocable auto suite) : name(name) {
            test_runner.add(detail::test_objects::Suite{suite, name});
        }
    };
}