//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "serialization.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <map>

#include "log.h"

#include "r_textures.h"
#include "system_list.h"
#include "controller_list.h"

#define PROJECT_DIR "***REMOVED***"

using namespace Verse;

namespace {
    bool write_to_project = false;
}

void Serialization::loadYAML(str name, YAML::Node &file) {
    str path = "res/" + name + ".yaml";
    
    //Check if the file exists
    std::filesystem::path f{path};
    if (not std::filesystem::exists(f)) {
        log::error("The file that you attempted to read (" + path + ") does not exist");
        return;
    }
    
    //Check for syntax errors
    try {
        file = YAML::LoadFile(path);
    } catch(const YAML::ParserException& e) {
        log::error("Error Parsing YAML (" + path + "): " + e.what());
    }
}

void Serialization::writeYAML(str name, YAML::Node &file) {
    str path = "res/" + name + ".yaml";
#ifdef __APPLE__
#ifdef DEBUG
    if (write_to_project)
        path = PROJECT_DIR + path;
#endif
#endif
    
    //Check if the file exists
    std::filesystem::path f{path};
    if (not std::filesystem::exists(f)) {
        log::error("The file that you attempted to write (" + path + ") does not exist");
        return;
    }
    
    std::ofstream fout(path);
    fout << file;
}

void Serialization::destroyYAML(str name) {
    str path = "res/" + name + ".yaml";
    
    std::filesystem::path f{path};
    
    //Check if the file exists
    if (not std::filesystem::exists(f)) {
        log::error("The YAML file that you attempted to remove (" + path + ") does not exist");
        return;
    }
    
    std::filesystem::remove(f);
}

void Serialization::appendYAML(str name, str key, YAML::Node &file, bool overwrite) {
    YAML::Node file_to_modify;
    loadYAML(name, file_to_modify);
    
    if (file_to_modify[key] and not overwrite) {
        log::warn("Attempted to write over existing YAML data with overwrite set to false (res/serialization/" + name + ".yaml). No changes were made.");
        return;
    }
    
    file_to_modify[key] = file;
    writeYAML(name, file_to_modify);
}

void Serialization::appendYAML(str name, std::vector<str> key, YAML::Node &file, bool overwrite) {
    if (key.size() == 1) {
        appendYAML(name, key[0], file);
        return;
    }
    
    YAML::Node file_to_modify;
    loadYAML(name, file_to_modify);
    
    YAML::Node temp_node[key.size()];
    
    if (not file_to_modify[key[0]]) {
        file_to_modify[key[0]] = file;
        writeYAML(name, file_to_modify);
        return;
    }
    
    if (file_to_modify[key[0]] and not overwrite) {
        log::warn("Attempted to write over existing YAML data with overwrite set to false (res/serialization/" + name + ".yaml). No changes were made.");
        return;
    }
    
    temp_node[0] = file_to_modify[key[0]];
    
    for (int i = 1; i <= key.size(); i++) {
        if (temp_node[i-1].IsMap() and temp_node[i-1][key[i]]) {
            temp_node[i] = temp_node[i-1][key[i]];
            continue;
        }
        
        if (not temp_node[i-1].IsMap())
            temp_node[i-1].reset();
        
        temp_node[i-1] = file;
        
        for (int j = i-1; j > 0; j -= 1) {
            temp_node[j-1][key[j]] = temp_node[j];
        }
        
        file_to_modify[key[0]] = temp_node[0];
        writeYAML(name, file_to_modify);
        return;
    }
}

//IMPORTANT: You need to specify (str) to call these functions, otherwise it will default to the other one
void Serialization::appendYAML(str name, str key, str value, bool overwrite) {
    YAML::Node n = YAML::Load(value);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, str value, bool overwrite) {
    YAML::Node n = YAML::Load(value);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, std::vector<str> vec, bool overwrite) {
    str y = "";
    for (str s : vec)
        y += "  - " + s + "\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, std::vector<str> vec, bool overwrite) {
    str y = "";
    for (str s : vec)
        y += "  - " + s + "\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}
//----------------------

void Serialization::appendYAML(str name, str key, int num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, int num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, std::vector<int> vec, bool overwrite) {
    str y = "";
    for (int v : vec)
        y += "  - " + std::to_string(v) + "\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, std::vector<int> vec, bool overwrite) {
    str y = "";
    for (int v : vec)
        y += "  - " + std::to_string(v) + "\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, float num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, float num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, bool b, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(b));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, bool b, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(b));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, Vec2 vec, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Vec2 vec, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, std::vector<Vec2> vec, bool overwrite) {
    str y = "";
    for (Vec2 v : vec)
        y += "  - [" + std::to_string(v.x) + "," + std::to_string(v.y) + "]\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, std::vector<Vec2> vec, bool overwrite) {
    str y = "";
    for (Vec2 v : vec)
        y += "  - [" + std::to_string(v.x) + "," + std::to_string(v.y) + "]\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, Rect2 rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," + std::to_string(rect.w) + "," + std::to_string(rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Rect2 rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," + std::to_string(rect.w) + "," + std::to_string(rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, Vec2f vec, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Vec2f vec, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(vec.x) + "," + std::to_string(vec.y) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, str key, Rect2f rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," + std::to_string(rect.w) + "," + std::to_string(rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Rect2f rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," + std::to_string(rect.w) + "," + std::to_string(rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::removeYAML(str name, str key) {
    YAML::Node file_to_modify;
    loadYAML(name, file_to_modify);
    
    if (file_to_modify[key]) {
        file_to_modify.remove(key);
        writeYAML(name, file_to_modify);
    }
}


void Serialization::removeYAML(str name, std::vector<str> key) {
    if (key.size() == 1) {
        removeYAML(name, key[0]);
        return;
    }
    
    YAML::Node file_to_modify;
    loadYAML(name, file_to_modify);
    
    YAML::Node temp_node[key.size()];
    if (file_to_modify[key[0]]) {
        temp_node[0] = file_to_modify[key[0]];
    }
    
    for (int i = 1; i < key.size(); i++) {
        try {
            if (not temp_node[i-1][key[i]])
                return;
            temp_node[i] = temp_node[i-1][key[i]];
        } catch (const YAML::Exception &e) {
            log::error(e.what());
            return;
        }
    }
    
    temp_node[key.size() - 2].remove(key[key.size() - 1]);
    writeYAML(name, file_to_modify);
}

void Serialization::loadScene(str name, Scene *s, Config &c) {
    YAML::Node data;
    str path = "data/scenes/" + name;
    Serialization::loadYAML(path, data);
    
    
    //SCENE DATA
    //--------------------------------------
    if(not data["scene"]) {
        log::error("You tried to load the scene (" + name + ") but it has no \'scene\' data");
        return;
    }
    if (not (data["scene"]["name"] and data["scene"]["size"])) {
        log::error("You tried to load the scene (" + name + ") but it has no valid \'scene\' data (name and size)");
        return;
    }
    s->name = data["scene"]["name"].as<str>();
    s->size = data["scene"]["size"].as<Vec2>();
    
    s->checkpoints = {};
    if (data["scene"]["spawn"]) {
        if (data["scene"]["spawn"].IsSequence()) {
            for (Vec2 sp : data["scene"]["spawn"].as<std::vector<Vec2>>()) {
                s->checkpoints.push_back(sp);
            }
        } else {
            s->checkpoints.push_back(data["scene"]["spawn"].as<Vec2>());
        }
    } else {
        s->checkpoints.push_back(Vec2(0,0));
    }
        
    //--------------------------------------
    
    
    //ENTITIES
    //--------------------------------------
    if(not (data["entities"] and data["entities"].IsMap())) {
        log::warn("You tried to load the scene (" + name + ") but it has no entities");
        return;
    }
    
    for(YAML::const_iterator i=data["entities"].begin(); i != data["entities"].end(); i++) {
        str entity_name = i->first.as<str>();
        YAML::Node entity = i->second;
        
        //Create entity
        EntityID eid = s->createEntity(entity_name);
        
        //Add components
        Serialization::loadComponentsFromYAML(eid, entity_name, entity, s, c);
    }
    //--------------------------------------
}

EntityID Serialization::loadPlayer(Scene *s, Config &c) {
    YAML::Node data;
    str path = "data/player";
    Serialization::loadYAML(path, data);
    
    if(not data["player"]) {
        log::error("You tried to load the player but the file has no data");
        return -1;
    }
    
    YAML::Node player = data["player"];
    
    EntityID eid = s->createEntity("player");
    Serialization::loadComponentsFromYAML(eid, "player", player, s, c);
    
#ifdef PLAYER
    s->addComponent<Component::Player>(eid);
#endif
    
    return eid;
}

void Serialization::loadComponentsFromYAML(EntityID eid, str entity_name, YAML::Node &entity, Scene *s, Config &c) {
#ifdef TEXTURE
        if (entity["texture"]) {
            Component::Texture* texture = s->addComponent<Component::Texture>(eid);
            if (not entity["texture"]["res"]) {
                log::error("You created a texture component for " + entity_name + " but it has no res for the texture");
                s->removeEntity(eid);
                return;
            }
            texture->res = entity["texture"]["res"].as<str>();
            Graphics::Texture::loadTexture(texture->res, texture);
            if (entity["texture"]["transform"])
                texture->transform = entity["texture"]["transform"].as<Rect2>();
            if (entity["texture"]["offset"]) {
                if (entity["texture"]["offset"].IsSequence()) {
                    texture->offset = entity["texture"]["offset"].as<std::vector<Vec2>>();
                } else {
                    texture->offset.push_back(entity["texture"]["offset"].as<Vec2>());
                }
            } else {
                texture->offset.push_back(Vec2(0,0));
            }
            if (entity["texture"]["layer"]) {
                if (entity["texture"]["layer"].IsSequence()) {
                    texture->layer = entity["texture"]["layer"].as<std::vector<int>>();
                } else {
                    texture->layer.push_back(entity["texture"]["layer"].as<int>());
                }
            } else {
                texture->layer.push_back(0);
            }
        }
#endif
#ifdef ANIMATION
        if (entity["animation"]) {
            Component::Animation* animation = s->addComponent<Component::Animation>(eid);
            ui16 default_fps = (entity["animation"]["default_fps"]) ? entity["animation"]["default_fps"].as<ui16>() : 3;
            if (not (entity["animation"]["frames"] and entity["animation"]["frames"].IsMap())) {
                log::error("You created an animation component for " + entity_name + " but it has no frames");
                s->removeEntity(eid);
                return;
            }
            for(YAML::const_iterator i=entity["animation"]["frames"].begin(); i != entity["animation"]["frames"].end(); i++) {
                str animation_name = i->first.as<str>();
                
                if (i->second.IsSequence()) {
                    animation->frames[animation_name].index = i->second.as<std::vector<ui16>>();
                    
                    std::vector<ui16> ms(animation->frames[animation_name].index.size(), (ui16)round(1000.0f / (float)default_fps));
                    animation->frames[animation_name].ms = ms;
                    
                    continue;
                }
                
                if (i->second.IsMap()) {
                    if (not (i->second["index"] and i->second["index"].IsSequence()))
                        continue;
                    animation->frames[animation_name].index = i->second["index"].as<std::vector<ui16>>();
                    
                    if (not i->second["fps"]) {
                        std::vector<ui16> ms(animation->frames[animation_name].index.size(), (ui16)round(1000.0f / (float)default_fps));
                        animation->frames[animation_name].ms = ms;
                        continue;
                    }
                    if (i->second["fps"].IsScalar()) {
                        std::vector<ui16> ms(animation->frames[animation_name].index.size(), (ui16)round(1000.0f / i->second["fps"].as<float>()));
                        animation->frames[animation_name].ms = ms;
                        continue;
                    }
                    std::vector<ui16> ms = i->second["fps"].as<std::vector<ui16>>();
                    std::transform(ms.begin(), ms.end(), ms.begin(), [](ui16 &c){ return (ui16)round(1000.0f / (float)c); });
                    animation->frames[animation_name].ms = ms;
                }
            }
            animation->curr_key = (entity["animation"]["curr_key"]) ? entity["animation"]["curr_key"].as<str>() : animation->frames.begin()->first;
            animation->size = (entity["animation"]["size"]) ? entity["animation"]["size"].as<int>() : 1;
        }
#endif
#ifdef ACTOR
        if (entity["actor"]) {
            Component::Actor* actor = s->addComponent<Component::Actor>(eid);
            if (entity["actor"]["controller"]) {
                actor_move_func move = &System::Actor::move;
                if (entity["actor"]["controller"].as<str>() == "player") {
                    actor->controller = PLAYER_CONTROLLER;
                    actor->damage = PLAYER_DAMAGE;
                }
                if (entity["actor"]["controller"].as<str>() == "free")
                    actor->controller = FREE_ACTOR_CONTROLLER;
            }
            if (entity["actor"]["max_move_speed"])
                actor->max_move_speed = entity["actor"]["max_move_speed"].as<int>();
            if (entity["actor"]["max_fall_speed"])
                actor->max_fall_speed = entity["actor"]["max_fall_speed"].as<int>();
            if (entity["actor"]["acc_ground"])
                actor->acc_ground = entity["actor"]["acc_ground"].as<int>();
            if (entity["actor"]["friction_ground"])
                actor->friction_ground = entity["actor"]["friction_ground"].as<int>();
            if (entity["actor"]["has_gravity"])
                actor->has_gravity = entity["actor"]["has_gravity"].as<bool>();
            if (entity["actor"]["collision_mask"]) {
                if (entity["actor"]["collision_mask"].IsScalar()) {
                    auto it = std::find(Component::c_layers_name.begin(),
                                        Component::c_layers_name.end(),
                                        entity["actor"]["collision_mask"].as<str>());
                    if (it != Component::c_layers_name.end())
                        actor->collision_mask.set(std::distance(Component::c_layers_name.begin(), it));
                } else {
                    for (str m : entity["actor"]["collision_mask"].as<std::vector<str>>()) {
                        auto it = std::find(Component::c_layers_name.begin(),
                                            Component::c_layers_name.end(), m);
                        if (it != Component::c_layers_name.end())
                            actor->collision_mask.set(std::distance(Component::c_layers_name.begin(), it));
                    }
                }
            }
        }
#endif
#ifdef TILEMAP
        if (entity["tilemap"]) {
            Component::Tilemap* tilemap = s->addComponent<Component::Tilemap>(eid);
            if (not entity["tilemap"]["tiles"]) {
                log::error("You created a tilemap component for " + entity_name + " but it has no tile attribute");
                s->removeEntity(eid);
                return;
            }
            tilemap->tile_res = entity["tilemap"]["tiles"].as<str>();
            tilemap->tiles = System::Tilemap::load(tilemap->tile_res);
            if (not entity["tilemap"]["res"]) {
                log::error("You created a tilemap component for " + entity_name + " but it has no res for the texture");
                s->removeEntity(eid);
                return;
            }
            tilemap->res = (entity["tilemap"]["res"].IsScalar()) ?
                            std::vector<str>({entity["tilemap"]["res"].as<str>()}) :
                            entity["tilemap"]["res"].as<std::vector<str>>();
            Graphics::Texture::loadTexture(tilemap->res, tilemap);
            if (entity["tilemap"]["pos"])
                tilemap->pos = entity["tilemap"]["pos"].as<Vec2>();
            if (entity["tilemap"]["tex_size"])
                tilemap->tex_size = entity["tilemap"]["tex_size"].as<Vec2>();
            tilemap->layer = (entity["tilemap"]["layer"]) ? entity["tilemap"]["layer"].as<int>() : 0;
        }
#endif
#ifdef COLLIDER
        if (entity["collider"]) {
            Component::Collider* collider = s->addComponent<Component::Collider>(eid);
            if (entity["collider"].IsMap()) {
                if (entity["collider"]["transform"])
                    collider->transform = entity["collider"]["transform"].as<Rect2>();
                collider->layer = Component::ColliderLayers::GROUND;
                if (entity["collider"]["layer"]) {
                    auto it = std::find(Component::c_layers_name.begin(),
                                        Component::c_layers_name.end(),
                                        entity["collider"]["layer"].as<str>());
                    if (it != Component::c_layers_name.end())
                        collider->layer = std::distance(Component::c_layers_name.begin(), it);
                }
            } else {
                #ifdef TILEMAP
                if (entity["collider"].as<str>() == "tile") {
                    Component::Tilemap* tilemap = s->getComponent<Component::Tilemap>(eid);
                    if (not (entity["tilemap"]["pos"] and tilemap != nullptr)) {
                        log::error("You tried to add a tilemap collider for " + entity_name + ", but it doesn't have a tilemap component");
                        s->removeEntity(eid);
                        return;
                    }
                    collider->transform = Rect2(tilemap->pos, System::Tilemap::calculateSize(tilemap));
                    collider->layer = Component::ColliderLayers::GROUND;
                }
                #endif
            }
        }
#endif
#ifdef LIGHT
        if (entity["light"]) {
            Component::Light* light = s->addComponent<Component::Light>(eid);
            if (entity["light"]["pos"])
                light->pos = entity["light"]["pos"].as<Vec2>();
            if (entity["light"]["radius"])
                light->radius = entity["light"]["radius"].as<int>();
        }
#endif
#ifdef CAMERA
        if (entity["camera"]) {
            Component::Camera* camera = s->addComponent<Component::Camera>(eid);
            if (entity["camera"]["pos"])
                camera->pos = entity["camera"]["pos"].as<Vec2f>();
            camera->bounds = Rect2(0,0,0,0);
            if (entity["camera"]["bounds"]) {
                if (entity["camera"]["bounds"].IsMap()) {
                    camera->bounds = entity["camera"]["bounds"].as<Rect2>();
                } else {
                    if (entity["camera"]["bounds"].as<str>() == "scene")
                        camera->bounds = Rect2(s->size * 0.5f, s->size);
                    if (entity["camera"]["bounds"].as<str>() == "none")
                        camera->bounds = Rect2(0,0,0,0);
                }
            }
            if (entity["camera"]["controller"]) {
                if (entity["camera"]["controller"].as<str>() == "actor")
                    camera->controller = CAMERA_ACTOR_CONTROLLER;
                if (entity["camera"]["controller"].as<str>() == "free")
                    camera->controller = CAMERA_FREE_CONTROLLER;
            }
            System::Camera::init(camera);
        }
#endif
#ifdef FIRE
    if (entity["fire"]) {
        Component::Fire* fire = s->addComponent<Component::Fire>(eid);
        if (entity["fire"]["transform"])
            fire->transform = entity["fire"]["transform"].as<Rect2>();
        if (entity["fire"]["offset"])
            fire->offset = entity["fire"]["offset"].as<Vec2>();
        if (entity["fire"]["dir"])
            fire->dir = entity["fire"]["dir"].as<Vec2>();
        if (entity["fire"]["fps"])
            fire->fps = (ui8)entity["fire"]["fps"].as<int>();
        if (entity["fire"]["freq"])
            fire->freq = entity["fire"]["freq"].as<float>();
        if (entity["fire"]["octaves"])
            fire->octaves = (ui8)entity["fire"]["octaves"].as<int>();
        if (entity["fire"]["seed"])
            fire->seed = (ui32)entity["fire"]["seed"].as<int>();
        if (not entity["fire"]["res"]) {
            log::error("You created a fire component for " + entity_name + " but it has no res for the texture");
            s->removeEntity(eid);
            return;
        }
        fire->layer = (entity["fire"]["layer"]) ? entity["fire"]["layer"].as<int>() : 0;
        fire->flame_tex_res = entity["fire"]["res"].as<str>();
        Graphics::Texture::loadTexture(fire->flame_tex_res, fire->flame_tex);
        System::Fire::init(fire);
    }
#endif
#ifdef SCENE_TRANSITION
    if (entity["scene_transition"]) {
        Component::SceneTransition* transition = s->addComponent<Component::SceneTransition>(eid);
        if (not entity["scene_transition"]["scene"]) {
            log::error("You created a scene transition component for " + entity_name + " but it has no scene url");
            s->removeEntity(eid);
            return;
        }
        transition->scene_name = entity["scene_transition"]["scene"].as<str>();
        transition->to_pos =  entity["scene_transition"]["pos"] ? entity["scene_transition"]["pos"].as<Vec2>() : Vec2(0,0);
    }
#endif
}

void Serialization::saveScene(Scene *s, Config &c, bool to_proj) {
    write_to_project = to_proj;
    for (EntityID e : SceneView<>(*s)) {
        if (s->getName(e) == "player")
            return;
        Serialization::saveComponentsToYAML(e, s, c);
    }
}

void Serialization::saveComponentsToYAML(EntityID eid, Scene *s, Config &c) {
    str path = "data/scenes/" + s->name;
    std::vector<str> key = {"entities", s->getName(eid), "", ""};
    //log::info("%s, %s, %s, %s", key[0].c_str(), key[1].c_str(), key[2].c_str(), key[3].c_str());
    
    Component::Collider* col = s->getComponent<Component::Collider>(eid);
    Component::Texture* tex = s->getComponent<Component::Texture>(eid);
    Component::Animation* anim = s->getComponent<Component::Animation>(eid);
    Component::Tilemap* tile = s->getComponent<Component::Tilemap>(eid);
    Component::Actor* actor = s->getComponent<Component::Actor>(eid);
    Component::Light* light = s->getComponent<Component::Light>(eid);
    Component::Camera* cam = s->getComponent<Component::Camera>(eid);
    Component::Fire* fire = s->getComponent<Component::Fire>(eid);
    Component::SceneTransition* trans = s->getComponent<Component::SceneTransition>(eid);
    Component::Player* player = s->getComponent<Component::Player>(eid);
    
    if (col != nullptr) {
        key[2] = "collider";
        
        if (tile == nullptr) {
            key[3] = "transform";
            appendYAML(path, key, col->transform, true);
            
            key[3] = "layer";
            appendYAML(path, key, (str)Component::c_layers_name[col->layer], true);
        } else {
            std::vector<str> tile_key = {key[0], key[1], key[2]};
            appendYAML(path, tile_key, "tile");
        }
    }
    
    if (tex != nullptr) {
        key[2] = "texture";
        
        key[3] = "res";
        appendYAML(path, key, (str)tex->res, true);
        
        key[3] = "transform";
        appendYAML(path, key, tex->transform, true);
        
        key[3] = "layer";
        if (tex->layer.size() == 1)
            appendYAML(path, key, tex->layer[0], true);
        else
            appendYAML(path, key, tex->layer, true);
        
        key[3] = "offset";
        if (tex->offset.size() == 1) {
            if (tex->offset[0] != Vec2(0,0))
                appendYAML(path, key, tex->offset[0], true);
        } else {
            appendYAML(path, key, tex->offset, true);
        }
    }
    
    if (anim != nullptr) {
        //TODO: Animation
    }
    
    if (tile != nullptr) {
        key[2] = "tilemap";
        
        key[3] = "res";
        if (tile->res.size() == 1)
            appendYAML(path, key, (str)tile->res[0], true);
        else
            appendYAML(path, key, tile->res, true);
        
        key[3] = "tiles";
        appendYAML(path, key, (str)tile->tile_res, true);
        
        key[3] = "pos";
        appendYAML(path, key, tile->pos, true);
        
        key[3] = "tex_size";
        appendYAML(path, key, tile->tex_size, true);
        
        key[3] = "layer";
        appendYAML(path, key, tile->layer, true);
    }
    
    if (actor != nullptr) {
        key[2] = "actor";
    }
    
    if (light != nullptr) {
        key[2] = "light";
    }
    
    if (cam != nullptr) {
        key[2] = "camera";
    }
    
    if (fire != nullptr) {
        key[2] = "fire";
    }
    
    if (trans != nullptr) {
        key[2] = "scene_transition";
    }
    
    if (player != nullptr) {
        key[2] = "player";
    }
}
