# `ecs`

## [`systems`](https://github.com/josekoalas/fresa/blob/main/core/system.h)

A system is a part of **fresa** that has a certain structure and is used to change how the engine behaves. By creating different functions, a system can execute code at the start, at the end or in the update loop. This is a key part of the entity-component-system architecture, but can also be used for systems that don't interact with the ecs directly, such as the job system. You can define a system in a struct-like fashion:

```cpp
struct SystemA {
    inline static System<SystemA> system;
    static void init() { /* ... */ }
}
```

There are two ways to register a system inside the manager, either by using `system::add()` or by creating a static member of type `System<>`, which will call add automaticaly on initialization. It is important to note that for systems to be registered, their header file must be included in at least one source file.

Systems can contain these functions:

- **`init()`**: called once in the engine initialization, in order of priority.
- **`update()`**: called each simulation frame the update loop, in order of priority.
- **`stop()`**: called once in the engine shutdown, in reverse order of priority.

An example of a system can be as follows:

```cpp
int count;

struct CountingSystem {
    inline static System<CountingSystem> system;

    static void init() { count = 0; }
    static void update() { count++; }
    static void stop() { log::info("count: {}", count); }
}
```

## [`entities`](https://github.com/josekoalas/fresa/blob/main/core/ecs.h)

In this ecs system, entities are just an id that can be used to access components from a pool. They are composed of an index and a version, both of them 16 bits, creating a total of 32 bits for the entity id, being the lower 16 bits the index and the upper 16 bits the version. The version is incremented each time the entity id is reused by a new entity.

```cpp
ecs::EntityID e = id(0, 0); //: create an entity from an index and a version
ecs::Index i = index(e); //: get the entity index
ecs::Version v = version(e); //: get the entity version
constexpr ecs::EntityID invalid_entity = id(-1, 0); //: this is an invalid entity defined for checking if an entity id is valid
```

## [`components`](https://github.com/josekoalas/fresa/blob/main/core/ecs.h)

Components are structs of data asociated to entities. Theoretically, components can be any type of data, even fundamental types such as `int`. However, for clarity, it is recommended to make each component type a struct. Entities can't have more than one component of the same type.

```cpp
struct PositionComponent {
    float x;
    float y;
};
```

These components are stored in `ComponentPool` objects, that function similar to a sparse allocator. Inside it there is a sparse array of all entity ids that map to a dense array of packed component data. There is actually a third array, with the same layout of the dense one, but this one mapping back to the position inside the sparse array. This is done for faster lookup in exchange for a slightly higher memory footprint.

## [`scenes`](https://github.com/josekoalas/fresa/blob/main/core/ecs.h)

The scene object exists to manage entities and component pools. It has a hash map of component pools that uses a constexpr type hash as the key. To get a component pool of a specific type from this map use `scene.cpool<Component>()`. If a component pool doesn't exist, this function will create it.

For regular usage, you can use the dedicated functions to manage entities:

```cpp
ecs::Scene scene; //: create a scene
ecs::EntityID e = scene.add(PositionComponent{1.0f, 2.0f}); //: create an entity with a position component
ecs::EntityID e = scene.add(ComponentA{}, ComponentB{}); //: create an entity with multiple components
scene.remove(e); //: remove the entity
auto position = scene.get<PositionComponent>(e); //: get the position component of the entity
```

The last line indicates how to get a component from an entity. The result is a `std::optional<>` that is null if the entity doesn't have the component. If it does have it, then the optional contains a reference to the underlying component in the sparse set.

## [`views`](https://github.com/josekoalas/fresa/blob/main/core/ecs.h)

Views are a way to iterate over entities and components. They will iterate over all entities that have **all** the components in the view. This is key for writing systems that operate on entities.

```cpp
auto view = ecs::View<PositionComponent>(scene);
for (auto [e, position] : view()) {
    //...
}
```

The view range can accessed by using the operator `()`, using a range-based for loop. It will decompose in a tuple of the entity id and the components currently iterating. A view can be defined for multiple components.

```cpp
for (auto [e, a, b] : ecs::View<ComponentA, ComponentB>(scene)()) {
    //...
}
```

You can read and modify the component values inside of the for loop, but the entity id is read only. Please note that unlike other range based for loops, no `&` is written before the square brackets, but the compontents inside are still references. This has to do with how `ranges::zip` works.

There is the possibility to iterate over just the data, without the entity id, by using the `data()` function.

```cpp
for (auto &position : ecs::View<ComponentPosition>(scene).data()) {
    //...
}
for (auto [a, b] : ecs::View<ComponentA, ComponentB>(scene).data()s) {
    //...
}
```