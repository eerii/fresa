//* event_tests
#ifdef FRESA_ENABLE_TESTS

#include "unit_test.h"
#include "events.h"

namespace test
{
    using namespace fresa;

    inline TestSuite event_tests("events", []{
        "add an event callback"_test = [&]{
            events::Event<int> e;
            auto id = e.add([](int i){});
            return expect(id != events::invalid_callback);
        };

        "remove an event callback"_test = [&]{
            events::Event<int> e;
            auto id = e.add([](int i){});
            e.remove(id);
            return expect(e.count() == 0);
        };

        "reset all event callbacks"_test = [&]{
            events::Event<int> e;
            e.add([](int i){});
            e.add([](int i){});
            e.add([](int i){});
            ui32 count_before_reset = e.count();
            e.reset();
            return expect(count_before_reset == 3 and e.count() == 0);
        };

        "publish an event"_test = [&]{
            events::Event<int> e;
            int i = 0;
            e.add([&](int j){ i = j; });
            e.publish(1);
            return expect(i == 1);
        };

        "publish an event with multiple callbacks"_test = [&]{
            events::Event<int> e;
            int a = 0;
            int b = 0;
            e.add([&](int i){ a = i; });
            e.add([&](int i){ b = i; });
            e.publish(1);
            return expect(a == 1 and b == 1);
        };

        "publish an event with removed callbacks"_test = [&]{
            events::Event<int> e;
            int a = 0;
            int b = 0;
            auto A = e.add([&](int i){ a = i; });
            auto B = e.add([&](int i){ b = i; });
            e.remove(B);
            e.publish(1);
            return expect(a == 1 and b == 0);
        };
    });
}

#endif