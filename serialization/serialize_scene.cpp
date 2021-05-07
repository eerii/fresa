//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "serialize_scene.h"
#include "r_textures.h"
#include "system_list.h"
#include <map>

using namespace Verse;

void Serialization::loadScene(str name, Scene &s, Config &c) {
    YAML::Node data;
    str path = "scenes/" + name;
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
            //Component::Animation* animation = s.addComponent<Component::Animation>(eid);
            //TODO: ANIMATION
            /*if (entity["texture"]["transform"])
                texture->transform = entity["texture"]["transform"].as<Rect2>();*/
        }
#endif
#ifdef ACTOR
        if (entity["actor"]) {
            //Component::Actor* actor = s.addComponent<Component::Actor>(eid);
            //TODO: ACTOR
            /*if (entity["texture"]["transform"])
                texture->transform = entity["texture"]["transform"].as<Rect2>();*/
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
            //Component::Camera* camera = s.addComponent<Component::Camera>(eid);
            /*TODO: CAMERA*/
        }
#endif
    }
    //--------------------------------------
}
