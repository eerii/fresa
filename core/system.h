//* system
//      manages the engine's subsystems, allowing for creation, iteration with priority, and destruction in order

#include "fresa_types.h"
#include "log.h"
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
    inline void stop() {
        while (not manager.stop.empty()) {
            log::debug("stopping system '{}'", manager.stop.top().name);
            manager.stop.top().f();
            manager.stop.pop();
        }
    }
}