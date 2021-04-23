//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "ecs.h"

#define logComponentID(x) log::debug("%s ID: %d", #x, Component::getID<Component::x>())

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
    COMPONENTS
}
//-------------------------------------
