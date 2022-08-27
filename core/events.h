//* events
//      simple event system
#pragma once

#include "std_types.h"
#include "strong_types.h"

#include <mutex>

namespace fresa::events
{
    //* callback id
    using CallbackID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Incrementable, strong::Hashable>;
    constexpr CallbackID invalid_callback = 0;

    //* event
    template <typename ... T>
    struct Event {
        //* constructors: no copy, just move
        Event() = default;
        Event(const Event &other) = delete;
        Event(Event &&other) : Event() { *this = std::move(other); }
        Event &operator=(const Event &other) = delete;
        Event &operator=(Event &&other) = default;

        //---

        //* callback
        //      function that takes arguments of type T... which is executed when the event is published

        //: callback type
        using Callback = std::function<void(T&& ...)>;

        //: callback data
        std::unordered_map<CallbackID, std::unique_ptr<Callback>> callbacks;
        CallbackID cb_counter = invalid_callback;
        std::mutex cb_mutex;

        //: add a callback
        CallbackID addHandler(Callback c) {
            std::lock_guard<std::mutex> lock(cb_mutex);
            callbacks[++cb_counter] = std::make_unique<Callback>(c);
            return cb_counter;
        }

        //---

        //* manage

        //: add
        CallbackID add(const Callback &h) { return addHandler(h); }
        
        //: remove
        void remove(CallbackID id) {
            std::lock_guard<std::mutex> lock(cb_mutex);
            if (callbacks.contains(id))
                callbacks.erase(id);
        }

        //: reset
        void reset() {
            std::lock_guard<std::mutex> lock(cb_mutex);
            callbacks.clear();
        }

        //: count
        ui32 count() {
            std::lock_guard<std::mutex> lock(cb_mutex);
            return callbacks.size();
        }

        //---

        //* publish

        void publish(T&& ... args) {
            std::lock_guard<std::mutex> lock(cb_mutex);
            for (auto &[id, c] : callbacks)
                c->operator()(std::forward<T>(args)...);
        }
    };
}