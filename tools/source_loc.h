//* source_location
//      provides the name of the file and the line number of the function calling it
//      standard in c++20, but compiler support is lacking, so an alternative is also provided
#pragma once

#ifdef __cpp_lib_source_location
    #include <source_location>
    namespace fresa::detail
    {
        using source_location = std::source_location;
    }
#else 
    namespace fresa::detail
    {
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
    }
#endif

