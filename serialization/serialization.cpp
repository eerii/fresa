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

#include "project_dir.h"

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

void Serialization::appendYAML(str name, str key, std::vector<ui16> vec, bool overwrite) {
    str y = "";
    for (ui16 v : vec)
        y += "  - " + std::to_string(v) + "\n";
    YAML::Node n = YAML::Load(y);
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, std::vector<ui16> vec, bool overwrite) {
    str y = "";
    for (ui16 v : vec)
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
    YAML::Node n = YAML::Load("[" + std::to_string(*rect.x) + "," + std::to_string(*rect.y) + "," + std::to_string(*rect.w) + "," + std::to_string(*rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Rect2 rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(*rect.x) + "," + std::to_string(*rect.y) + "," + std::to_string(*rect.w) + "," + std::to_string(*rect.h) + "]");
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
    YAML::Node n = YAML::Load("[" + std::to_string(*rect.x) + "," + std::to_string(*rect.y) + "," + std::to_string(*rect.w) + "," + std::to_string(*rect.h) + "]");
    appendYAML(name, key, n, overwrite);
}

void Serialization::appendYAML(str name, std::vector<str> key, Rect2f rect, bool overwrite) {
    YAML::Node n = YAML::Load("[" + std::to_string(*rect.x) + "," + std::to_string(*rect.y) + "," + std::to_string(*rect.w) + "," + std::to_string(*rect.h) + "]");
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
        Serialization::loadComponentsFromYAML(eid, entity, s, c);
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
    Serialization::loadComponentsFromYAML(eid, player, s, c);
    
#ifdef USE_C_PLAYER
    s->addComponent<Component::Player>(eid);
#endif
    s->addComponent<Component::State>(eid);
    
    return eid;
}

void Serialization::loadComponentsFromYAML(EntityID eid, YAML::Node &entity, Scene *s, Config &c) {
    if (entity["texture"])
        System::Texture::load(eid, entity, s, c);
    if (entity["animation"])
        System::Animation::load(eid, entity, s, c);
    if (entity["tilemap"])
        System::Tilemap::load(eid, entity, s, c);
    if (entity["actor"])
        System::Actor::load(eid, entity, s, c);
    if (entity["collider"])
        System::Collider::load(eid, entity, s, c);
    if (entity["camera"])
        System::Camera::load(eid, entity, s, c);
    if (entity["light"])
        System::Light::load(eid, entity, s, c);
    if (entity["timer"])
        System::Timer::load(eid, entity, s, c);
    if (entity["patrol"])
        System::Patrol::load(eid, entity, s, c);
    if (entity["scene_transition"])
        System::SceneTransition::load(eid, entity, s, c);
#ifdef USE_C_FIRE
    if (entity["fire"])
        System::Fire::load(eid, entity, s, c);
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
    Component::SceneTransition* trans = s->getComponent<Component::SceneTransition>(eid);
    
    if (col != nullptr)
        System::Collider::save(col, path, key, tile != nullptr);
    
    if (tex != nullptr)
        System::Texture::save(tex, path, key);
    
    if (anim != nullptr)
        System::Animation::save(anim, path, key);
    
    if (tile != nullptr)
        System::Tilemap::save(tile, path, key);
    
    if (actor != nullptr) {
        key[2] = "actor";
    }
    
    if (light != nullptr) {
        key[2] = "light";
    }
    
    if (cam != nullptr) {
        key[2] = "camera";
    }
    
    if (trans != nullptr) {
        key[2] = "scene_transition";
    }
    
#ifdef USE_C_FIRE
    Component::Fire* fire = s->getComponent<Component::Fire>(eid);
    if (fire != nullptr) {
        key[2] = "fire";
    }
#endif
    
#ifdef USE_C_PLAYER
    Component::Player* player = s->getComponent<Component::Player>(eid);
    if (player != nullptr) {
        key[2] = "player";
    }
#endif
}
