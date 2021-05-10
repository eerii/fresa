//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include <filesystem>
#include <iostream>
#include <fstream>
#include <map>

#include "serialization.h"

#include "r_textures.h"
#include "system_list.h"
#include "controller_list.h"

using namespace Verse;

void Serialization::initYAML() {
    std::filesystem::create_directories("res/data/save");
    std::filesystem::create_directories("res/data/scenes");
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
    
    if (file_to_modify[key]) {
        //The entry already exists
        if (overwrite) {
            file_to_modify[key] = file;
        } else {
            log::warn("Attempted to write over existing YAML data with overwrite set to false (res/serialization/" + name + ".yaml). No changes were made.");
        }
    } else {
        //The entry does not exist
        file_to_modify[key] = file;
    }
    
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
    if (file_to_modify[key[0]]) {
        if (overwrite) {
            temp_node[0] = file_to_modify[key[0]];
        } else {
            log::warn("Attempted to write over existing YAML data with overwrite set to false (res/serialization/" + name + ".yaml). No changes were made.");
            return;
        }
    } else {
        file_to_modify[key[0]] = file;
        writeYAML(name, file_to_modify);
        return;
    }
    
    for (int i = 1; i <= key.size(); i++) {
        if (temp_node[i-1].IsScalar()) {
            temp_node[i-1].reset();
        } else if (temp_node[i-1][key[i]]) {
            temp_node[i] = temp_node[i-1][key[i]];
            continue;
        }
        
        temp_node[key.size() - 1] = file;
        for (int j = (int)key.size() - 1; j > 0; j -= 1) {
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
//----------------------

void Serialization::appendYAML(str name, str key, int num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, int num, bool overwrite) {
    YAML::Node n = YAML::Load(std::to_string(num));
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
            if (temp_node[i-1][key[i]]) {
                temp_node[i] = temp_node[i-1][key[i]];
            } else {
                return;
            }
        } catch (const YAML::Exception &e) {
            log::error(e.what());
            return;
        }
    }
    
    temp_node[key.size() - 2].remove(key[key.size() - 1]);
    writeYAML(name, file_to_modify);
}

void Serialization::loadScene(str name, Scene &s, Config &c) {
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
    s.name = data["scene"]["name"].as<str>();
    s.size = data["scene"]["size"].as<Vec2>();
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
        EntityID eid = s.createEntity(entity_name);
        
        //Add components
        Serialization::loadComponentsFromYAML(eid, entity_name, entity, s, c);
    }
    //--------------------------------------
}

EntityID Serialization::loadPlayer(Scene &s, Config &c) {
    YAML::Node data;
    str path = "data/player";
    Serialization::loadYAML(path, data);
    
    if(not data["player"]) {
        log::error("You tried to load the player but the file has no data");
        return -1;
    }
    
    YAML::Node player = data["player"];
    
    EntityID eid = s.createEntity("player");
    Serialization::loadComponentsFromYAML(eid, "player", player, s, c);
    
    return eid;
}

void Serialization::loadComponentsFromYAML(EntityID eid, str entity_name, YAML::Node &entity, Scene &s, Config &c) {
#ifdef TEXTURE
        if (entity["texture"]) {
            Component::Texture* texture = s.addComponent<Component::Texture>(eid);
            if (not entity["texture"]["res"]) {
                log::error("You created a texture component for " + entity_name + " but it has no res for the texture");
                s.removeEntity(eid);
                return;
            }
            Graphics::Texture::loadTexture(entity["texture"]["res"].as<str>(), texture);
            if (entity["texture"]["transform"])
                texture->transform = entity["texture"]["transform"].as<Rect2>();
            if (entity["texture"]["offset"])
                texture->offset = entity["texture"]["offset"].as<Vec2>();
        }
#endif
#ifdef ANIMATION
        if (entity["animation"]) {
            Component::Animation* animation = s.addComponent<Component::Animation>(eid);
            if (not (entity["animation"]["frames"] and entity["animation"]["frames"].IsMap())) {
                log::error("You created an animation component for " + entity_name + " but it has no frames");
                s.removeEntity(eid);
                return;
            }
            for(YAML::const_iterator i=entity["animation"]["frames"].begin(); i != entity["animation"]["frames"].end(); i++) {
                str animation_name = i->first.as<str>();
                std::vector<ui16> frames = i->second.as<std::vector<ui16>>();
                animation->frames[animation_name] = frames;
            }
            if (entity["animation"]["curr_key"]) {
                animation->curr_key = entity["animation"]["curr_key"].as<str>();
            } else {
                animation->curr_key = animation->frames.begin()->first;
            }
            if (entity["animation"]["size"]) {
                animation->size = entity["animation"]["size"].as<int>();
            } else {
                animation->size = animation->calculate_size();
            }
        }
#endif
#ifdef ACTOR
        if (entity["actor"]) {
            Component::Actor* actor = s.addComponent<Component::Actor>(eid);
            if (entity["actor"]["controller"]) {
                if (entity["actor"]["controller"].as<str>() == "player")
                    actor->controller = PLAYER_CONTROLLER;
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
                actor->friction_ground = entity["actor"]["friction_ground"].as<float>();
        }
#endif
#ifdef TILEMAP
        if (entity["tilemap"]) {
            Component::Tilemap* tilemap = s.addComponent<Component::Tilemap>(eid);
            if (not entity["tilemap"]["tiles"]) {
                log::error("You created a tilemap component for " + entity_name + " but it has no tile attribute");
                s.removeEntity(eid);
                return;
            }
            if (not entity["tilemap"]["res"]) {
                log::error("You created a tilemap component for " + entity_name + " but it has no res for the texture");
                s.removeEntity(eid);
                return;
            }
            tilemap->tiles = System::Tilemap::load(entity["tilemap"]["tiles"].as<str>());
            Graphics::Texture::loadTexture(entity["tilemap"]["res"].as<str>(), tilemap);
            if (entity["tilemap"]["pos"])
                tilemap->pos = entity["tilemap"]["pos"].as<Vec2>();
            if (entity["tilemap"]["tex_size"])
                tilemap->tex_size = entity["tilemap"]["tex_size"].as<Vec2>();
        }
#endif
#ifdef COLLIDER
        if (entity["collider"]) {
            Component::Collider* collider = s.addComponent<Component::Collider>(eid);
            if (entity["collider"].IsMap()) {
                if (entity["collider"]["transform"])
                    collider->transform = entity["collider"]["transform"].as<Rect2>();
            } else {
                #ifdef TILEMAP
                if (entity["collider"].as<str>() == "tile") {
                    Component::Tilemap* tilemap = s.getComponent<Component::Tilemap>(eid);
                    if (not (entity["tilemap"]["pos"] and tilemap != nullptr)) {
                        log::error("You tried to add a tilemap collider for " + entity_name + ", but it doesn't have a tilemap component");
                        s.removeEntity(eid);
                        return;
                    }
                    collider->transform = Rect2(tilemap->pos, System::Tilemap::calculateSize(tilemap));
                }
                #endif
            }
        }
#endif
#ifdef LIGHT
        if (entity["light"]) {
            Component::Light* light = s.addComponent<Component::Light>(eid);
            if (entity["light"]["pos"])
                light->pos = entity["light"]["pos"].as<Vec2>();
            if (entity["light"]["radius"])
                light->radius = entity["light"]["radius"].as<int>();
        }
#endif
#ifdef CAMERA
        if (entity["camera"]) {
            Component::Camera* camera = s.addComponent<Component::Camera>(eid);
            if (entity["camera"]["pos"])
                camera->pos = entity["camera"]["pos"].as<Vec2f>();
            if (entity["camera"]["bounds"]) {
                if (entity["camera"]["bounds"].IsMap()) {
                    camera->bounds = entity["camera"]["bounds"].as<Rect2>();
                } else {
                    if (entity["camera"]["bounds"].as<str>() == "scene")
                        camera->bounds = Rect2(s.size * 0.5f, s.size);
                    if (entity["camera"]["bounds"].as<str>() == "none")
                        camera->bounds = Rect2(0,0,0,0);
                }
            } else {
                camera->bounds = Rect2(0,0,0,0);
            }
            if (entity["camera"]["focus"]) {
                camera->focus_size = entity["camera"]["focus"].as<Vec2>();
            } else {
                camera->focus_size = Vec2(0,0);
            }
            if (entity["camera"]["look_ahead"])
                camera->look_ahead = entity["camera"]["look_ahead"].as<int>();
            if (entity["camera"]["la_speed"])
                camera->la_speed = entity["camera"]["la_speed"].as<int>();
            if (entity["camera"]["controller"]) {
                if (entity["camera"]["controller"].as<str>() == "actor")
                    camera->controller = CAMERA_ACTOR_CONTROLLER;
            }
            System::Camera::init(camera);
        }
#endif
#ifdef FIRE
    if (entity["fire"]) {
        Component::Fire* fire = s.addComponent<Component::Fire>(eid);
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
            s.removeEntity(eid);
            return;
        }
        Graphics::Texture::loadTexture(entity["fire"]["res"].as<str>(), fire->flame_tex);
        System::Fire::init(fire);
    }
#endif
}
