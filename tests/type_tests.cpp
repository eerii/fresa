//* types_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "atomic_queue.h"
#include "constexpr_for.h"
#include "bidirectional_map.h"

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
            str s = "MeRmElAdA!";
            return expect(lower(s) == "mermelada!");
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

    //* atomic queue
    inline TestSuite atomic_queue_test("atomic_queue", []{
        "atomic queue push"_test = []{
            auto q = AtomicQueue<int>();

            bool before = q.size() == 0;

            std::jthread t1([&]{ q.push(1); });
            std::jthread t2([&]{ q.push(2); });
            t1.join();
            t2.join();

            return expect(before and q.size() == 2);
        };

        "atomic queue pop"_test = []{
            auto q = AtomicQueue<int>();
            q.push(1);

            std::atomic<int> value = 0;
            std::jthread t([&]{
                auto v = q.pop(); 
                if (v)
                    value = v.value();
            });
            t.join();

            return expect(value == 1 and q.size() == 0);
        };

        "atomic queue clear"_test = []{
            auto q = AtomicQueue<int>();
            q.push(1);
            q.push(2);
            q.push(3);

            std::jthread t([&]{ q.clear(); });
            t.join();

            return expect(q.size() == 0);
        };
    });

    //* constexpr for
    inline TestSuite constexpr_for_test("constexpr_for", []{
        "constexpr integral for"_test = []{
            int sum = 0;
            for_<0, 10>([&](int i){ sum += i; });
            return expect(sum == 45);
        };
        "constexpr parameter pack for"_test = []{
            int sum = 0;
            for_([&](int i){ sum += i; }, 2, 4, 8, 16);
            return expect(sum == 30);
        };
        "constexpr tuple-like for"_test = []{
            bool result = true;
            for_([&](auto const& v){ result = result and v; }, std::make_tuple(true, 1, "hello"));
            return expect(result);
        };
    });

    //* bidirectional map
    inline TestSuite bidirectional_map_test("bidirectional_map", []{
        "adding"_test = []{
            BiMap<int, str> m;
            m.add(1, "a");
            m.add(1, "b");
            m.add(1, "c");
            m.add(2, "b");

            return expect(m.a_to_b == decltype(m.a_to_b)({{1, "a"}, {1, "b"}, {1, "c"}, {2, "b"}}) and
                          m.b_to_a == decltype(m.b_to_a)({{"a", 1}, {"b", 1}, {"c", 1}, {"b", 2}}));
        };

        "initializer constructor"_test = []{
            BiMap<int, str> m{{
                {1, "a"},
                {1, "b"},
                {1, "c"},
                {2, "b"}
            }};

            return expect(m.a_to_b == decltype(m.a_to_b)({{1, "a"}, {1, "b"}, {1, "c"}, {2, "b"}}) and
                          m.b_to_a == decltype(m.b_to_a)({{"a", 1}, {"b", 1}, {"c", 1}, {"b", 2}}));
        };

        "removing"_test = []{
            BiMap<int, str> m({ {1, "a"}, {1, "b"}, {1, "c"}, {2, "b"} });
            m.remove_b("b");

            return expect(m.a_to_b == decltype(m.a_to_b)({{1, "a"}, {1, "c"}}) and
                          m.b_to_a == decltype(m.b_to_a)({{"a", 1}, {"c", 1}}));
        };

        "remove specific pair"_test = []{
            BiMap<int, str> m({ {1, "a"}, {1, "b"}, {1, "c"}, {2, "b"} });
            m.remove(1, "b");
            return expect(m.a_to_b == decltype(m.a_to_b)({{1, "a"}, {1, "c"}, {2, "b"}}) and
                          m.b_to_a == decltype(m.b_to_a)({{"a", 1}, {"c", 1}, {"b", 2}}));
        };

        "get b from a"_test = []{
            BiMap<int, str> m({ {1, "a"}, {1, "b"}, {1, "c"}, {2, "b"} });
            return expect((m.get_b(1) | ranges::to_vector) == std::vector<str>{"a", "b", "c"});
        };

        "get a from b"_test = []{
            BiMap<int, str> m({ {1, "a"}, {1, "b"}, {1, "c"}, {2, "b"} });
            return expect((m.get_a("b") | ranges::to_vector) == std::vector<int>{1, 2});
        };
    });
}

#endif