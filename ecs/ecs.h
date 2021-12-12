//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "dtypes.h"
#include "reflection.h"
#include "ecs_description.h"

#include <bitset>

#define INVALID_ENTITY Entity::createID(EntityIndex(-1), 0)

//---ECS--
//      Very (very) basic Entity Component System, which is formed by:
//      - Components (a struct of data)
//      - Systems (a function that updates the data of those componentes)
//      - Entities (just an id)
//      - Scenes (list of entities and their associated components)
//      It is very barebones and should be used only for small projects, something more robust like https://github.com/skypjack/entt is more
//      appropiate for larger projects. This is meant to be educational only

namespace Fresa
{
    typedef ui32 EntityID;
    const EntityID MAX_ENTITIES = 4096;

    typedef ui8 ComponentID;
    const ComponentID MAX_COMPONENTS = 32;

    typedef std::bitset<MAX_COMPONENTS> Signature;
}

namespace Fresa::Entity
{
    typedef ui16 EntityIndex;
    typedef ui16 EntityVersion;

    inline EntityID createID(EntityIndex index, EntityVersion version) { return ((EntityID)index << 16) | ((EntityID)version); };
    inline EntityIndex getIndex(EntityID eid) { return eid >> 16; };
    inline EntityVersion getVersion(EntityID eid) { return (EntityVersion)eid; };

    inline bool isValid(EntityID eid) { return (eid >> 16) != EntityIndex(-1); };
}

namespace Fresa::Component
{
    inline ComponentID component_counter = 0;
    
    template<typename C, std::enable_if_t<Reflection::is_reflectable<C> && Reflection::is_in_variant<C, ComponentType>::value, bool> = true>
    ComponentID getID() {
        static ComponentID id_ = component_counter++;
        if (id_ >= MAX_COMPONENTS)
            throw std::runtime_error("Increase max component capacity");
        return id_;
    }
}
