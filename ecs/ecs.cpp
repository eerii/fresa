//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#include "ecs.h"
#include "log.h"

#include "component_list.h"

using namespace Fresa;

ComponentID Component::component_counter = 0;

template <class T>
ComponentID Component::getID() {
    static ComponentID id = Component::component_counter++;
    return id;
}

void Component::registerComponents() {
    //---Component registration---
    //      Right now components are created using variants, but I'd like to use the reflection system and extend this function
    //      to create a registerComponent(Component) expression that will handle it automatically and more gracefully than right now
    for_<std::variant_size_v<ComponentType>>([&](auto i) {
        ComponentID cid = Component::getID<std::variant_alternative_t<i.value, ComponentType>>();
        log::debug("%s, ID: %d", component_names[i.value].c_str(), cid);
    });
}
