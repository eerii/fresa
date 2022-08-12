# working with entities

Now you are ready to use the ecs features of fresa, creating and managing a set of entities and components.

## create a component

In **fresa** all entities are made of a collection of components. A component can be any arbitrary data structure, even a simple type. However, it is recommended to create specific structs for each component. An entity can't have two components of the same type. An example of how to define a component is as follows:

```cpp
struct PositionComponent {
    float x;
    float y;
};
```

## adding an entity

To add an entity you need to create a scene, which is a container for all the component pools and entities. Then, you can use the method `scene.add(...)` to add an entity to the scene with the specified list of componets. `scene.add()` can be called with zero, one or multiple components. In this case we are going to import the `"ecs.h"` header and then add an entity with the position component we initialized before.

```cpp
#include "ecs.h"

//...

ecs::Scene scene;
ecs::EntityID entity = scene.add(PositionComponent{1.0f, 3.0f});
```

## accessing components

You can get a specific component from an entity using `scene.get<C>(entity)`. It returns an optional encapsulating a reference to the component value. If the entity doesn't have the component, it returns an empty optional. Following the previous example you can modify the position of the entity like so:

```cpp
auto& position = scene.get<PositionComponent>(entity);
if (position.has_value())
    position.value().x = 2.0f;
```

Using an optional is verbose in this case, however, there is another way to efficiently access components, and that is with views.

## views

A view is an iterator over a range of entities with one or more components. For example, you can iterate over all entities with `PositionComponent`, or all entities with both `PositionComponent` and `VelocityComponent`. These are the building blocks of systems.

Lets say we want to move all entities in the x direction. We can create a view that does that like so:

```cpp
for (auto [entity, position] : ecs::View<PositionComponent>(scene)()) {
    position.x += 1.0f;
}
```

The view can be used directly inside a for loop (calling it with `()` at the end). It will return a tuple of values with the entity id and the component references you asked for, in this case `PositionComponent`. You can operate on the components easily. This will run for all entities that match the criteria. Lets see another example:

```cpp
struct ColorComponent {
    str color_name;
};

for (auto [entity, position, color] : ecs::View<PositionComponent, ColorComponent>(scene)()) {
    log::info("the entity at ({}, {}) has the color {}", position.x, position.y, color.color_name);
}
```

## full example

Lets create a final example, this time, we will find the entity that is the furthest from the origin. We will use a system `init()` function to run this code for now.

```cpp title="example.h"
#pragma once
#include "system.h"
#include "ecs.h
#include "log.h"

struct PositionComponent {
    float x;
    float y;
};

namespace fresa
{
    struct SomeSystem {
        inline static System<SomeSystem> system;

        static void init() {
            ecs::Scene scene;
            scene.add(PositionComponent{1.0f, 3.0f});
            scene.add(PositionComponent{2.0f, 1.5f});
            scene.add(PositionComponent{-7.0f, -3.0f}); //: this is the furthest from the origin
            scene.add(PositionComponent{-1.2f, -0.5f});
            
            ecs::Index furthest;
            auto norm = [](const PositionComponent& p) { return std::sqrt(p.x * p.x + p.y * p.y); };
            for (auto [entity, position] : ecs::View<PositionComponent>(scene)()) {
                PositionComponent& max_pos = scene.get<PositionComponent>(ecs::id(furthest, 0)).value();
                if (norm(position) > norm(max_pos))
                    furthest = entity;
            }

            log::info("the furthest entity is {}", furthest.value);
        }
    };
}
```