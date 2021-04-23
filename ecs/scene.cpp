//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "scene.h"

using namespace Verse;

EntityID Scene::createEntity(std::string name) {
    if (!free_entities.empty()) {
        Entity::EntityIndex new_index = free_entities.front();
        free_entities.pop_back();
        
        EntityID new_id = Entity::createID(new_index, Entity::getVersion(entities[new_index]));
        entities[new_index] = new_id;
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

std::string Scene::getName(EntityID eid) {
    return entity_names[Entity::getIndex(eid)];
}

void Scene::setSize(Vec2 scene_size) {
    size = scene_size;
}

void Scene::addConnectedScene(Scene *scene, Vec2 pos) {
    connected_scenes.push_back(scene);
    pos_connected_scenes.push_back(pos);
}
