//* types_tests

#include "unit_test.h"
#include "fresa_types.h"

namespace test
{
    using namespace fresa;

    //* strings
    inline TestSuite strings_test("strings", []{
        "lowercase conversion"_test = []{
            return expect(lower("FReSa") == "fresa");
        };
        "lowercase literal"_test = []{
            return expect("JoSEkOALaS"_lower == "josekoalas");
        };
        "lowercase with string variable"_test = []{
            str s = "AguACAtE!";
            return expect(lower(s) == "aguacate!");
        };
        "split string with commas"_test = []{
            auto s = split("a,b,c,d", ',') | ranges::to_vector;
            return expect(s == std::vector<str_view>{"a", "b", "c", "d"});
        };
        "split string with extra spaces"_test = []{
            auto s = split("hey   hi hello    !", ' ') | ranges::to_vector;
            return expect(s == std::vector<str_view>{"hey", "hi", "hello", "!"});
        };
    });

    //* coroutines
    namespace detail
    {
        using namespace coroutines;

        Future<int> counter_yield() {
            for (int i = 0 ;; i++)
                co_yield i;
        }

        Future<int> counter_yield_with_return() {
            for (int i = 0; i < 5; i++)
                co_yield i;
            co_return 0;
        }

        Future<int> coroutine_return() {
            co_return 256;
        }

        str name = "bob";
        Future<void> coroutine_void() {
            name = "amy";
            co_return;
        }
    }

    inline TestSuite coroutines_test("coroutines", []{
        "coroutine with yield"_test = []{
            std::array<int, 5> a;
            auto c = detail::counter_yield();
            c.handle.resume();
            for (int i = 0; i < 5; i++)  {
                a[i] = c.get();
                c.handle();
            }
            c.handle.destroy();
            return expect(a == std::to_array<int>({0, 1, 2, 3, 4}));
        };
        "coroutine with yield and return"_test = []{
            std::array<int, 5> a;
            auto c = detail::counter_yield_with_return();
            c.handle.resume();
            int i = 0;
            while (not c.handle.done()) {
                a[i++] = c.get();
                c.handle();
            }
            c.handle.destroy();
            return expect(a == std::to_array<int>({0, 1, 2, 3, 4}));
        };
        "coroutine with return"_test = []{
            auto c = detail::coroutine_return();
            c.handle.resume();
            int result = c.get();
            c.handle.destroy();
            return expect(result == 256);
        };
        "coroutine void"_test = []{
            auto c = detail::coroutine_void();
            c.handle.resume();
            c.handle.destroy();
            return expect(detail::name == "amy");
        };
    });
}