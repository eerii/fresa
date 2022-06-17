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

        Future<Promise, int> counter_await() {
            auto p = co_await AwaitablePromise<Promise<int>>{};
            for (int i = 0 ;; i++) {
                p->value = i;
                co_await std_::suspend_always{};
            }
        }

        Future<Promise, int> counter_yield() {
            for (int i = 0 ;; i++)
                co_yield i;
        }

        Future<Promise, int> counter_yield_implicit_return() {
            for (int i = 0; i < 5; i++)
                co_yield i;
        }
    }

    inline TestSuite coroutines_test("coroutines", []{
        "coroutine with await"_test = []{
            std::array<int, 5> a;
            auto h = detail::counter_await().handle;
            h.resume();
            for (int i = 0; i < 5; i++) {
                a[i] = h.promise().value;
                h();
            }
            h.destroy();
            return expect(a == std::to_array<int>({0, 1, 2, 3, 4}));
        };
        "coroutine with yield"_test = []{
            std::array<int, 5> a;
            auto h = detail::counter_yield().handle;
            h.resume();
            for (int i = 0; i < 5; i++)  {
                a[i] = h.promise().value;
                h();
            }
            h.destroy();
            return expect(a == std::to_array<int>({0, 1, 2, 3, 4}));
        };
        "coroutine with yield and implicit return"_test = []{
            std::array<int, 5> a;
            auto h = detail::counter_yield_implicit_return().handle;
            h.resume();
            int i = 0;
            while (not h.done()) {
                a[i++] = h.promise().value;
                h();
            }
            h.destroy();
            return expect(a == std::to_array<int>({0, 1, 2, 3, 4}));
        };
    });
}