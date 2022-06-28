//* system_tests
#ifdef ENABLE_TESTS

#include "unit_test.h"
#include "system.h"

namespace test
{
    using namespace fresa;
    
    inline TestSuite system_test("systems", []{
        struct SystemA {
            static void init() {};
            static void update() {};
            static void stop() {};
        };

        struct SystemB {
            static void init() {};
            static void update() {};
            static void stop() {};
        };

        struct SystemC {
            static void init() {};
            static void stop() {};
        };

        "before adding systems"_test = []{
            return expect(system::manager.update.size() == 0 and system::manager.stop.size() == 0);
        };
        "adding system A with default update priority (100)"_test = []{
            system::add(SystemA{});
            return expect(system::manager.update.size() == 1 and system::manager.stop.size() == 1);
        };
        "adding system B with higher update priority (0)"_test = []{
            system::add(SystemB{}, system::SYSTEM_PRIORITY_FIRST);
            return expect(system::manager.update.size() == 2 and system::manager.stop.size() == 2);
        };
        "B has higher update priority than A"_test = []{
            return expect(system::manager.update.top().name == "SystemB");
        };
        "adding system C"_test = []{
            system::add(SystemC{});
            return expect(system::manager.update.size() == 2 and system::manager.stop.size() == 3);
        };
        "check stop order"_test = []{
            return expect(system::manager.stop.top().name == "SystemC");
        };
        "stop systems"_test = []{
            system::stop();
            return expect(system::manager.stop.size() == 0);
        };
    });
}

#endif