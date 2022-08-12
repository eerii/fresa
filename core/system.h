//* system
//      manages the engine's subsystems, allowing for creation, iteration with priority, and destruction in order
#pragma once

#include "std_types.h"
#include "type_name.h"
#include "log.h"

#include <queue>
#include <stack>

namespace fresa::system
{
    namespace concepts
    {
        //* system concept, requires an init function
        template <typename T>
        concept System = requires(T s) { T::init(); };

        //* system with a stop callback
        template <typename T>
        concept SystemWithStop = System<T> && requires(T s) { T::stop(); };

        //* system with simulation update
        template <typename T>
        concept SystemWithUpdate = System<T> and requires(T s) { T::update(); };
    }

    namespace detail
    {
        //* objects for the system update and stop lists
        //      a lower number means higher priority, so a system with priority 2 comes before a system with priority 4
        struct SystemObject {
            ui8 priority;
            str_view name;
            std::function<void()> f;
        };

        using SystemPriorityQueue = std::priority_queue<detail::SystemObject, std::deque<detail::SystemObject>, 
                                    decltype([](auto a, auto b){ return a.priority >= b.priority; })>;
    }

    //* system priorities
    enum SystemPriorities {
        SYSTEM_PRIORITY_FIRST = 0,
        SYSTEM_PRIORITY_DEFAULT = 100,
        SYSTEM_PRIORITY_LAST = 200,
    };

    //* system manager
    inline struct SystemManager {
        detail::SystemPriorityQueue init;
        detail::SystemPriorityQueue update;
        std::stack<detail::SystemObject> stop;
    } manager;

    //* register and initialize system
    inline void add(concepts::System auto s, ui8 priority = SYSTEM_PRIORITY_DEFAULT) {
        constexpr auto name = type_name<decltype(s)>();

        //: initialize system
        manager.init.push({priority, name, s.init});
        
        //: add to update list
        if constexpr (concepts::SystemWithUpdate<decltype(s)>)
            manager.update.push({priority, name, s.update});

        //: add to destructor list
        if constexpr (concepts::SystemWithStop<decltype(s)>)
            manager.stop.push({priority, name, s.stop});
    }

    //* init all systems in order
    inline void init() {
        while (not manager.init.empty()) {
            ::fresa::detail::log<"SYSTEM", LOG_DEBUG, fmt::color::medium_purple>("initializing system '{}'", manager.init.top().name);
            manager.init.top().f();
            manager.init.pop();
        }
    }

    //* clean all systems in inverse order when program stops
    inline void stop() {
        while (not manager.stop.empty()) {
            ::fresa::detail::log<"SYSTEM", LOG_DEBUG, fmt::color::medium_purple>("stopping system '{}'", manager.stop.top().name);
            manager.stop.top().f();
            manager.stop.pop();
        }
    }

    //* simulation update all systems in order
    inline void update() {
        auto queue = manager.update;
        while (not queue.empty()) {
            queue.top().f(); queue.pop();
        }
    }
}

//* system helper for registration
//      automatically adds a system to the manager, used like:
//
//      struct SomeSystem {
//          inline static System<SomeSystem> system;
//          static void init() {...}
//      }
//
//      it uses static initialization to register the system using `system::add`, which is then executed when calling `system::init` from engine
namespace fresa
{
    template <typename T, ui8 priority = system::SYSTEM_PRIORITY_DEFAULT>
    struct System {
        System() { system::add(T{}, priority); }
    };
}