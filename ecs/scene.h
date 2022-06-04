//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "ecs.h"
#include "cpool.h"
#include <map>

namespace Fresa
{
    //---Scene---
    //      It holds a entities with their signatures (associated components), as well as a component pool allocator.
    //      There are also scene properties, like it's name or size, which are useful to save here. It might be expanded in the future
    //      Finally, there are methods for adding and removing entities, as well as for adding and removing components to an entity
    struct Scene{
        Scene() = default;
        
        //: ECS
        std::vector<EntityID> entities;
        std::vector<Signature> mask;
        std::vector<std::string> entity_names;
        std::vector<Entity::EntityIndex> free_entities;
        
        std::vector<ComponentPool*> component_pools;
        
        //: Scene properties
        str name;
        
        //: Methods
        EntityID createEntity();
        EntityID createEntity(std::string name);
        void removeEntity(EntityID eid);
        
        template<typename C>
        C* addComponent(EntityID eid) {
            int cid = Component::getID<C>();
            
            if (component_pools.size() <= cid)
                component_pools.resize(cid + 1, nullptr);
            if (component_pools[cid] == nullptr)
                component_pools[cid] = new ComponentPool(sizeof(C));
            
            C* component = new (component_pools[cid]->get(Entity::getIndex(eid))) C();
            
            mask[Entity::getIndex(eid)].set(cid);
            return component;
        }
        
        template<typename C>
        C* getComponent(EntityID eid) {
            int cid = Component::getID<C>();
            
            if (!mask[Entity::getIndex(eid)].test(cid))
                return nullptr;
            
            C* component = static_cast<C*>(component_pools[cid]->get(Entity::getIndex(eid)));
            return component;
        }
        
        template<typename C>
        void removeComponent(EntityID eid) {
            removeComponent(eid, Component::getID<C>());
        }
        
        void removeComponent(EntityID eid, ComponentID cid);
        
        str getName(EntityID eid);
        Signature getMask(EntityID eid);
    };

    //---Scene view---
    //      Iterable type that can be used to loop over all entities in a scene that match a certain component mask, like:
    //      for (EntityID e : SceneView<Component>(scene)) ...
    template<typename... ComponentTypes>
    struct SceneView
    {
        SceneView(Scene &scene) : scene_ptr(&scene) {
            if (sizeof...(ComponentTypes) == 0) {
                all = true;
            }
            else {
                ComponentID component_ids[] = { 0, Component::getID<ComponentTypes>() ... };
                for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
                    signature.set(component_ids[i]);
            }
        }

        struct Iterator
        {
            Iterator(Scene* p_scene, Entity::EntityIndex p_index, Signature p_signature, bool p_all) :
            i_scene(p_scene), i_index(p_index), i_signature(p_signature), i_all(p_all) {}

            EntityID operator*() const {
                return i_scene->entities[i_index];
            }
        
            bool operator==(const Iterator& other) const {
                return i_index == other.i_index || i_index == i_scene->entities.size();
            }

            bool operator!=(const Iterator& other) const {
                return i_index != other.i_index && i_index != i_scene->entities.size();
            }
            
            bool validIndex() {
                return  Entity::isValid(i_scene->entities[i_index]) &&
                        (i_all || i_signature == (i_signature & i_scene->mask[i_index]));
            }

            Iterator& operator++() {
                do {
                    i_index++;
                } while (i_index < i_scene->entities.size() && !validIndex());
                return *this;
            }
            
            Scene* i_scene{ nullptr };
            Entity::EntityIndex i_index;
            Signature i_signature;
            bool i_all{ false };
        };

        const Iterator begin() const {
            int first_i = 0;
            while (first_i < scene_ptr->entities.size() &&
                  (signature != (signature & scene_ptr->mask[first_i]) ||
                   !Entity::isValid(scene_ptr->entities[first_i]))) {
                first_i++;
            }
            return Iterator(scene_ptr, first_i, signature, all);
        }
        
        const Iterator end() const {
            return Iterator(scene_ptr, Entity::EntityIndex(scene_ptr->entities.size()), signature, all);
        }
        
        Scene* scene_ptr{ nullptr };
        Signature signature;
        bool all{ false };
    };
    
    //---Scene registration---
    //      Scenes can be created using registerScene(), which adds them to a map that identifies them with an id
    //      This is a very inefficient allocator, but since the number of scenes should be small relative to everything else,
    //      it is fine for now. Definitely revisit in the future, maybe to add more functionality to registerScene (serialization)
    using SceneID = ui32;
    inline std::map<SceneID, Scene> scene_list;
    inline SceneID active_scene = 0;
    
    SceneID registerScene(str name);
}
