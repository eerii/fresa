//* fresa_test
//      basic unit testing framework with no macros
//      key idea from kris jusiak (https://www.youtube.com/watch?v=-qAXShy1xiE)
#pragma once

#include "fresa_types.h"
#include "log.h"
#ifdef __cpp_lib_source_location
    #include <source_location>
#endif

namespace fresa
{
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

        //* printable concept
        template <typename T>
        concept Printable = requires(std::ostream &os, T t) {
            { os << t } -> std::same_as<std::ostream&>;
        };

        //* test concept (printable and assignable)
        template <typename T, auto expression = []{}>
        concept Test = requires(T t) {
            { t.name } -> Printable;
            { t = expression } -> std::same_as<void>;
        };
    }

    //* test
    struct Test {
        str_view name;
        
        void operator=(std::invocable auto test) {
            log::test("Running {}", name);
            test();
        }
    };

    //* test literal operator
    constexpr detail::Test auto operator""_test(const char* name, std::size_t size) {
        return Test{.name = str_view{name, size}};
    }
}