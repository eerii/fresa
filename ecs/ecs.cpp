//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "ecs.h"
#include "log.h"

#include "component_list.h"

using namespace Verse;

//REGISTER COMPONENTS
//-------------------------------------
ComponentID Component::component_counter = 0;

template <class T>
ComponentID Component::getID() {
    static ComponentID id = Component::component_counter++;
    return id;
}

void Component::registerComponents() {
    /*for_<std::variant_size_v<ComponentType>>([&](auto i) {
        ComponentID cid = Component::getID<std::variant_alternative_t<i.value, ComponentType>>();
        log::debug("%s, ID: %d", component_names[i.value].c_str(), cid);
    });*/
    getID<Component::Collider>;
    getID<Component::CircleCollider>;
    getID<Component::Actor>;
    getID<Component::State>;
    getID<Component::Texture>;
    getID<Component::Animation>;
    getID<Component::Tilemap>;
    getID<Component::Text>;
    getID<Component::Camera>;
    getID<Component::Light>;
    getID<Component::Noise>;
    getID<Component::Timer>;
    getID<Component::Patrol>;
    getID<Component::SceneTransition>;
    getID<Component::Dialogue>;
    getID<Component::Player>;
}
//-------------------------------------
