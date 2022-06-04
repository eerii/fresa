//: fresa by jose pazos perez, licensed under GPLv3
//the event system was inspired by https://github.com/TheLartians/Observe

#pragma once

#include "types.h"

#include <mutex>

namespace Fresa::Event
{
    template <typename... Args> struct Event;
    template <typename... Args> struct SharedEvent;
    using HandlerID = ui32;

    //---Observer---
    //      Waits for an event to happen and executes a callback. Can be reasigned to another event and can de removed from an event
    struct Observer {
        //: Remove observer from event once destroyed
        struct Base {
            virtual ~Base() {}
        };
        
        //: Constructors
        Observer() {}
        Observer(Observer &&other) = default;
        template <typename L> Observer(L &&l) : data(new L(std::move(l))) {}
        
        //: Assignment
        Observer &operator=(const Observer &other) = delete;
        Observer &operator=(Observer &&other) = default;
        template <typename L> Observer &operator=(L &&l) {
            data.reset(new L(std::move(l)));
            return *this;
        }
        
        //: Start observing an event
        template <typename H, typename... Args> void observe(Event<Args...> &event, const H &handler) {
            data.reset(new typename Event<Args...>::Observer(event.createObserver(handler)));
        }
        
        //: Stop observing an event
        void reset() { data.reset(); }
        
        //: Check if it currently is observing an event
        explicit operator bool() const { return bool(data); }
        
        //: Observer data
        std::unique_ptr<Base> data;
    };
    
    //---Event---
    //      Create an event which you can emit and every handler (observer or callback) will execute the callback function associated
    template <typename... Args>
    struct Event {
        //---Handler---
        
        //: Type for this event handler
        using Handler = std::function<void(const Args &...)>;

        //: Storage for an event handler with it's callback
        struct StoredHandler {
            HandlerID id;
            std::shared_ptr<Handler> callback;
        };
        using HandlerList = std::vector<StoredHandler>;
    
        //: Handler data
        struct Data {
            HandlerID id_counter = 0;
            HandlerList observers;
            std::mutex observer_mutex;
        };
        std::shared_ptr<Data> data;
        
        //: Add a handler to the event
        HandlerID addHandler(Handler h) const {
            std::lock_guard<std::mutex> lock(data->observer_mutex);
            data->observers.emplace_back(StoredHandler{data->id_counter, std::make_shared<Handler>(h)});
            return data->id_counter++;
        }
        
        //---Observer---
        
        struct Observer : ::Fresa::Event::Observer::Base {
            //: Observer data pointer
            std::weak_ptr<Data> data;
            //: Handler id
            HandlerID id;
            
            //: Constructors
            Observer() {}
            Observer(const std::weak_ptr<Data> &_data, HandlerID _id) : data(_data), id(_id) {}
            
            Observer(Observer &&other) = default;
            Observer(const Observer &other) = delete;
            
            //: Assignment
            Observer &operator=(const Observer &other) = delete;
            Observer &operator=(Observer &&other) = default;
            
            //: Change the observed event (for the same type)
            void observe(const Event &event, const Handler &handler) {
                reset();
                *this = event.createObserver(handler);
            }
            
            //: Removes the handler from the event
            void reset() {
                if (auto d = data.lock()) {
                    std::lock_guard<std::mutex> lock(d->observer_mutex);
                    auto it = std::find_if(d->observers.begin(), d->observers.end(), [&](auto &o) { return o.id == id; });
                    if (it != d->observers.end()) {
                        d->observers.erase(it);
                    }
                }
                data.reset();
            }
            
            //: Destructor
            ~Observer() { reset(); }
        };
        
        //: Number of observers associated with the event
        size_t observerCount() const {
            std::lock_guard<std::mutex> lock(data->observer_mutex);
            return data->observers.size();
        }
        
        //---Handler management---
        
        //: Creates a new observer and passes a temporary handler to the event
        Observer createObserver(const Handler &h) const { return Observer(data, addHandler(h)); }
        
        //: Creates or removes a permanent handler to the event
        HandlerID callback(const Handler &h) const { return addHandler(h); }
        void removeCallback(HandlerID id) const { Observer(data, id).reset(); }
        
        //: Removes all handlers to the event
        void reset() const {
            std::lock_guard<std::mutex> lock(data->observer_mutex);
            data->observers.clear();
        }
        
        //---Emit---
        //      Calls the handlers connected to the event (in order of addition)
        void publish(Args... args) const {
            std::vector<std::weak_ptr<Handler>> handlers;
            {
                std::lock_guard<std::mutex> lock(data->observer_mutex);
                handlers.resize(data->observers.size());
                std::transform(data->observers.begin(), data->observers.end(), handlers.begin(),
                               [](auto &h) { return h.callback; });
            }
            for (auto &weak_callback : handlers) {
                if (auto callback = weak_callback.lock()) {
                    (*callback)(args...);
                }
            }
        }
        
        //---Constructors---
        Event() : data(std::make_shared<Data>()) {}
        Event(Event &&other) : Event() { *this = std::move(other); }
        
        //---Assignment---
        Event &operator=(Event &&other) {
            std::swap(data, other.data);
            return *this;
        }
        //: Prevent duplication of an event and its handlers, use SharedEvent for this functionality
        protected:
            Event(const Event &) = default;
            Event &operator=(const Event &) = default;
    };
    
    //---Shared event---
    //      An Event struct that can be copied and assigned
    template <typename... Args> struct SharedEvent : Event<Args...> {
        using Event<Args...>::Event;

        SharedEvent(const SharedEvent<Args...> &other) : Event<Args...>(other) {}

        SharedEvent &operator=(const SharedEvent<Args...> &other) {
            Event<Args...>::operator=(other);
            return *this;
        }
    };
    
    //---System events---
    void handleSystemEvents();
    inline Event<> event_quit;
    inline Event<bool> event_paused;
}
