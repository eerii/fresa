//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "ecs.h"
#include "log.h"

#define logComponentID(x) log::debug("%s ID: %d", #x, Component::getID<Component::x>()); \
                          names[Component::getID<Component::x>()] = #x;

using namespace Verse;

namespace {
    str names[MAX_COMPONENTS];
}

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

str Component::getName(ComponentID cid) {
    return names[cid];
}
