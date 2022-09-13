//* unit_test
//      basic unit testing framework with no macros
//      key idea from kris jusiak (https://www.youtube.com/watch?v=-qAXShy1xiE)
//      can be completely disabled from production code with the preprocessor macro FRESA_ENABLE_TESTS

//: example
//      TestSuite suite("Some tests", []{
//          "Test one"_test = []{ return expect(1 == 1); };      // passes
//          "Test two"_test = []{ return expect(1 == 2); };      // fails
//      });
//      ...
//      test_runner.run();
#pragma once
#ifdef FRESA_ENABLE_TESTS

#include "std_types.h"
#include "source_loc.h"
#include "log.h"

namespace fresa
{
    namespace concepts
    {
        //* expression concept
        //      can be evaluated to a boolean
        template <typename T>
        concept BooleanExpression = std::convertible_to<T, bool>;
    }

    namespace detail
    {
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
                    detail::log<"TEST", LogLevel::TEST, fmt::color::slate_gray>("running suite '{}'", suite.name);
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
                detail::log<"TEST PASSED", LogLevel::TEST, fmt::color::lime_green>("{}", name);
            } else {
                str_view file_name = t.location.file_name();
                file_name = file_name.substr(file_name.find_last_of("/") + 1);
                detail::log<"TEST FAILED", LogLevel::TEST, fmt::color::red>("{} ({}:{})", name, file_name, t.location.line());
            }
        }
    };

    //* test literal operator
    constexpr auto operator""_test(const char* name, std::size_t size) {
        return Test{.name = str_view{name, size}};
    }

    //* expect
    //      evaluates an expression from a test and returns the result as well as the code location
    constexpr auto expect(concepts::BooleanExpression auto expression, const detail::source_location &location = detail::source_location::current()) {
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

#endif