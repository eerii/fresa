# [`events`](https://github.com/josekoalas/fresa/blob/main/core/events.h)

Events are a way to communicate between systems. They are used to notify of changes and trigger callbacks associated with them. The event system in **fresa** is really simple, you just need to create an event object and subscribe functions to it.

## event object

An event is defined by `events::Event`. It can have zero or some template parameters, which are the types that will be passed to the subscriber functions. Creating an event is as simple as to define it.

```cpp
events::Event<> e;
events::Event<int, float> e;
```

## add a callback

To add a function to an event you may call the method `add()` with a suitable function (must match the event template parameters).

```cpp
events::Event<int, float> e;

//: with a lambda
e.add([](int i, float f) {
    log::info("event received: {} {}", i, f);
});

//: with a function
void callback(int i, float f) {
    log::info("event received: {} {}", i, f);
}
e.add(callback);
```

When adding a callback, the method will return an `events::CallbackID` handle, which can be stored to access the callback later (for example, to remove it).

```cpp
events::CallbackID id = e.add([](){});
```

## remove a callback

To remove a callback you may call the method `remove()` with the `events::CallbackID` handle.

```cpp
events::Event<> e;
events::CallbackID id = e.add([](){});
e.remove(id);
```

You can remove all callbacks with the method `reset()`.

```cpp
e.reset();
```

## publish an event

When an event is published, all callbacks are called with the event parameters. The `publish()` method takes the same parameters as the event template. Event callbacks are thread safe and there are no guarantees about the order in which they are called. If a callback is removed from the event, it will not be called when the event is published.

```cpp
events::Event<int, float> e;
events::CallbackID id = e.add([](int i, float f) {
    log::info("event received: {} {}", i, f);
});
e.publish(1, 2.0f);

// >> event received: 1 2.0
```