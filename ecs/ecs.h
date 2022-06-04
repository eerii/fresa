//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "types.h"

//: A component list is necessary to include all the component types and define a variant with all those types
//: You need to create a file component_list.h and add it to the project, the structure can be as follows:
//      #include <variant> /* For the list of types */
//      #include "..." /* Add here your component definitions */
//      namespace Fresa::Component {
//          using ComponentType = std::variant<...>; /* And here the name of the component classes */
//      }
//: The component classes need to be serialized in order to be reflectable, you can define them as:
//      struct SomeComponent {
//          Members(SomeComponent, something, another_thing, ...);
//          int something;
//          str another_thing; ...
//      };

#if __has_include("component_list.h")
    #include "component_list.h"
#else
    using ComponentType = std::variant<>;
#endif

#include <bitset>

#define INVALID_ENTITY Entity::createID(EntityIndex(-1), 0)

#define UPDATE_LIST(update_name, update_map, update_function) \
template<typename Object, UpdatePriorities priority = PRIORITY_FIRST> \
struct update_name { \
    struct exec_register { \
        exec_register() { \
            addToMultimap(update_map, priority, type_name<Object>(), Object::update_function); \
        } \
    }; \
    template<exec_register&> struct ref_it { }; \
    static exec_register register_object; \
    static ref_it<register_object> referrer; \
}; \
\
template<typename Object, UpdatePriorities priority> typename update_name<Object, priority>::exec_register \
update_name<Object, priority>::register_object;

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
    template <typename> struct component_tag {};
    
    template<typename C, std::enable_if_t<is_in_variant<C, ComponentType>::value, bool> = true>
    constexpr ComponentID getID() {
        constexpr ComponentID id_ = getVariantIndex<C, ComponentType>::value;
        static_assert(id_ < MAX_COMPONENTS, "Increase max component capacity");
        return id_;
    }
    
    //: Example of looping through components
    //      for_<Component::ComponentType>([](auto i){ using C = std::variant_alternative_t<i.value, Component::ComponentType>; ... });
}

namespace Fresa::System
{
    enum UpdatePriorities {
        //: Common priorities
        PRIORITY_FIRST = 0,
        PRIORITY_GUI = 31,
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
    inline void addToMultimap(std::multimap<UpdatePriorities, std::pair<std::string_view, std::function<void()>>> &map, UpdatePriorities priority,
                              std::string_view name, std::function<void()> update) {
        map.insert({ priority, std::pair<std::string_view, std::function<void()>>(name, update) });
    }
    
    inline std::multimap<UpdatePriorities, std::pair<std::string_view, std::function<void()>>> init_systems{};
    inline std::multimap<UpdatePriorities, std::pair<std::string_view, std::function<void()>>> physics_update_systems{};
    inline std::multimap<UpdatePriorities, std::pair<std::string_view, std::function<void()>>> render_update_systems{};
    
    //: Register the system when the template is instantiated, and adds it to the corresponding map of systems
    //      struct SomeSystem : PhysicsUpdate<SomeSystem, PRIORITY_MOVEMENT> {
    //          static void update();
    //      }
    UPDATE_LIST(SystemInit, init_systems, init)
    UPDATE_LIST(PhysicsUpdate, physics_update_systems, update)
    UPDATE_LIST(RenderUpdate, render_update_systems, render)
}
