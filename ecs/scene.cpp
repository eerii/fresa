//: fresa by jose pazos perez, licensed under GPLv3
#include "scene.h"

using namespace Fresa;

EntityID Scene::createEntity(std::string name) {
    if (not free_entities.empty()) {
        Entity::EntityIndex new_index = free_entities.front();
        free_entities.pop_back();
        
        EntityID new_id = Entity::createID(new_index, Entity::getVersion(entities[new_index]));
        entities[new_index] = new_id;
        entity_names[new_index] = name;
        return entities[new_index];
    }
    
    entities.push_back(Entity::createID(Entity::EntityIndex(entities.size()), 0));
    mask.push_back(Signature());
    entity_names.push_back(name);
    return entities.back();
}

EntityID Scene::createEntity() {
    return Scene::createEntity("");
}

void Scene::removeEntity(EntityID eid) {
    EntityID new_id = Entity::createID(Entity::EntityIndex(-1), Entity::getVersion(eid) + 1);
    entities[Entity::getIndex(eid)] = new_id;
    mask[Entity::getIndex(eid)].reset();
    free_entities.push_back(Entity::getIndex(eid));
}

void Scene::removeComponent(EntityID eid, ComponentID cid) {
    if (entities[Entity::getIndex(eid)] != eid)
        return;
    
    mask[Entity::getIndex(eid)].reset(cid);
}

str Scene::getName(EntityID eid) {
    return entity_names.at(Entity::getIndex(eid));
}

Signature Scene::getMask(EntityID eid) {
    return mask.at(Entity::getIndex(eid));
}

SceneID Fresa::registerScene(str name) {
    static SceneID id = 0;
    while (scene_list.find(id) != scene_list.end())
        id++;
    
    scene_list[id] = Scene{};
    scene_list[id].name = name;
    
    return id;
}
