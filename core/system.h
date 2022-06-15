//* system
//      ...

#include "fresa_types.h"
#include "log.h"
#include "unit_test.h"
#include <stack>

namespace fresa
{
    namespace detail
    {
        //* system concept, requires an initialacion and stop function
        template <typename T>
        concept TSystem = requires(T system) {
            T::init();
            T::stop();
        };

        //* system with simulation update
        template <typename T>
        concept TSystemUpdate = TSystem<T> and requires(T system) {
            T::update();
        };

        //* handle for a function with a name attached to it
        struct FunctionHandle {
            str_view name;
            void (*f) ();
        };
    }

    //* system manager
    inline struct SystemManager {
        std::vector<detail::FunctionHandle> update;
        std::stack<detail::FunctionHandle> stop;
    } systems;

    //* register and initialize system
    void registerSystem(detail::TSystem auto system) {
        constexpr auto name = type_name<decltype(system)>();
        log::debug("registering system '{}'", name);

        //* initialize system
        system.init();
        
        //* add to update list
        if constexpr (detail::TSystemUpdate<decltype(system)>)
            systems.update.push_back({name, system.update});

        //* add to destructor list
        systems.stop.push({name, system.stop});
    }

    //* clean all systems in inverse order when program stops
    void stopSystems() {
        while (not systems.stop.empty()) {
            log::debug("stopping system '{}'", systems.stop.top().name);
            systems.stop.top().f();
            systems.stop.pop();
        }
    }
}

namespace test
{
    using namespace fresa;
    inline TestSuite system_test("system", []{
        struct SystemA {
            static void init() {};
            static void update() {};
            static void stop() {};
        };

        struct SystemB {
            static void init() {};
            static void stop() {};
        };

        "before adding systems"_test = []{ return expect(systems.update.size() == 0 and systems.stop.size() == 0); };
        "adding system A"_test = []{
            registerSystem(SystemA{});
            return expect(systems.update.size() == 1 and systems.stop.size() == 1);
        };
        "adding system B"_test = []{
            registerSystem(SystemB{});
            return expect(systems.update.size() == 1 and systems.stop.size() == 2);
        };
        "check stop order"_test = []{
            return expect(systems.stop.top().name == "SystemB");
        };
        "stop systems"_test = []{
            stopSystems();
            return expect(systems.stop.size() == 0);
        };
    });
}