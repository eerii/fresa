//* system
//      manages the engine's subsystems, allowing for creation, iteration with priority, and destruction in order

#include "fresa_types.h"
#include "log.h"
#include "unit_test.h"
#include <queue>
#include <stack>

namespace fresa::system
{
    namespace detail
    {
        //* system concept, requires an initialacion and stop function
        template <typename T>
        concept TSystem = requires(T s) {
            T::init();
            T::stop();
        };

        //* system with simulation update
        template <typename T>
        concept TSystemUpdate = TSystem<T> and requires(T s) {
            T::update();
        };

        //* objects for the system update and stop lists
        //      a lower number means higher priority, so a system with priority 2 comes before a system with priority 4
        struct SystemUpdateObject {
            ui8 priority;
            str_view name;
            void (*f)();
        };
        struct SystemStopObject {
            str_view name;
            void (*f) ();
        };
    }

    //* system priorities
    enum SystemPriorities {
        SYSTEM_PRIORITY_FIRST = 0,
        SYSTEM_PRIORITY_DEFAULT = 100,
        SYSTEM_PRIORITY_LAST = 200,
    };

    //* system manager
    inline struct SystemManager {
        std::priority_queue<detail::SystemUpdateObject, std::deque<detail::SystemUpdateObject>, 
                            decltype([](auto a, auto b){ return a.priority >= b.priority; })> update;
        std::stack<detail::SystemStopObject> stop;
    } manager;

    //* register and initialize system
    void add(detail::TSystem auto s, ui8 priority = SYSTEM_PRIORITY_DEFAULT) {
        constexpr auto name = type_name<decltype(s)>();
        log::debug("registering system '{}'", name);

        //* initialize system
        s.init();
        
        //* add to update list
        if constexpr (detail::TSystemUpdate<decltype(s)>)
            manager.update.push({priority, name, s.update});

        //* add to destructor list
        manager.stop.push({name, s.stop});
    }

    //* clean all systems in inverse order when program stops
    void stop() {
        while (not manager.stop.empty()) {
            log::debug("stopping system '{}'", manager.stop.top().name);
            manager.stop.top().f();
            manager.stop.pop();
        }
    }
}

//* unit tests
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