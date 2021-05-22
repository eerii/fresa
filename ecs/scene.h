//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "ecs.h"
#include "cpool.h"

#include <vector>

namespace Verse
{
    struct Scene{
        Scene() = default;
        
        ~Scene() { } //Add here any code when scene is destroyed
        
        std::vector<EntityID> entities;
        std::vector<Signature> mask;
        std::vector<std::string> entity_names;
        std::vector<Entity::EntityIndex> free_entities;
        
        std::vector<ComponentPool*> component_pools;
        
        Vec2 size;
        str name;
        
        EntityID createEntity();
        EntityID createEntity(std::string name);
        void removeEntity(EntityID eid);
        
        template<class T>
        T* addComponent(EntityID eid) {
            int cid = Component::getID<T>();
            
            if (component_pools.size() <= cid)
                component_pools.resize(cid + 1, nullptr);
            if (component_pools[cid] == nullptr)
                component_pools[cid] = new ComponentPool(sizeof(T));
            
            T* component = new (component_pools[cid]->get(Entity::getIndex(eid))) T();
            
            mask[Entity::getIndex(eid)].set(cid);
            return component;
        }
        
        template<class T>
        T* getComponent(EntityID eid) {
            int cid = Component::getID<T>();
            
            if (!mask[Entity::getIndex(eid)].test(cid))
                return nullptr;
            
            T* component = static_cast<T*>(component_pools[cid]->get(Entity::getIndex(eid)));
            return component;
        }
        
        template<class T>
        void removeComponent(EntityID eid) {
            if (entities[Entity::getIndex(eid)] != eid)
                return;
            
            mask[Entity::getIndex(eid)].reset(Component::getID<T>());
        }
        
        str getName(EntityID eid);
    };

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
}
