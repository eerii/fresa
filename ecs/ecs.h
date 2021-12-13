//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "dtypes.h"
#include "reflection.h"
#include "component_list.h"

#include "log.h"

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

namespace Fresa::System
{
    enum UpdatePriorities {
        //: Common priorities
        PRIORITY_FIRST = 0,
        PRIORITY_LAST = 32,
        
        //: Physics priorities
        PRIORITY_MOVEMENT = 1,
        PRIORITY_CAMERA = 2,
        
        //: Render priorities
        PRIORITY_TEXTURE = 16,
        PRIORITY_TILEMAP = 17,
        PRIORITY_TEXT = 18,
    };
    
    //: Render and physics update systems
    inline std::multimap<UpdatePriorities, std::function<void()>> physics_update_systems{};
    inline std::multimap<UpdatePriorities, std::function<void()>> render_update_systems{};
    
    inline void addToMultimap(std::multimap<UpdatePriorities, std::function<void()>> &map, UpdatePriorities priority, std::function<void()> update) {
        map.insert({ priority, update });
    }
    
    //: Register the system when the template is instantiated, and adds it to the corresponding map of systems
    //      struct SomeSystem : PhysicsUpdate<SomeSystem, PRIORITY_MOVEMENT> {
    //          static void update();
    //      }
    template<typename Object, UpdatePriorities priority = PRIORITY_LAST>
    struct PhysicsUpdate {
        struct exec_register {
            exec_register() {
                addToMultimap(physics_update_systems, priority, Object::update);
            }
        };
        template<exec_register&> struct ref_it { };
        static exec_register register_object;
        static ref_it<register_object> referrer;
    };

    template<typename Object, UpdatePriorities priority> typename PhysicsUpdate<Object, priority>::exec_register
        PhysicsUpdate<Object, priority>::register_object;
    
    template<typename Object, UpdatePriorities priority = PRIORITY_LAST>
    struct RenderUpdate {
        struct exec_register {
            exec_register() {
                addToMultimap(render_update_systems, priority, Object::render);
            }
        };
        template<exec_register&> struct ref_it { };
        static exec_register register_object;
        static ref_it<register_object> referrer;
    };

    template<typename Object, UpdatePriorities priority> typename RenderUpdate<Object, priority>::exec_register
    RenderUpdate<Object, priority>::register_object;
}
