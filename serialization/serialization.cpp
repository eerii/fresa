//: fresa by jose pazos perez, licensed under GPLv3
#include "serialization.h"
#include "file.h"
#include "log.h"
#include <fstream>

using namespace Fresa;

int Serialization::getIndentation(const str &line) {
    size_t indentation = line.find_first_not_of(" ");
    indentation = indentation == std::string::npos ? 0 : indentation / 2;
    
    if (line.size() <= indentation * 2) return -1; //: Blank line
    if (line.at(indentation) == '#') return -1; //: Comment
    
    return (int)indentation;
}

void Serialization::loadComponents(const str &line, LoadState &state, SceneID scene, EntityID eid, int ind, int base_ind, bool add_components) {
    static str current_component = "";
    
    if (state == LOAD_COMPONENT_NAME or state == LOAD_COMPONENT_BODY) {
        state = (ind == base_ind) ? LOAD_COMPONENT_NAME : LOAD_COMPONENT_BODY;
    }
    
    //: Load component members
    if (state == LOAD_COMPONENT_BODY) {
        std::vector<str> item = split(line, ":");
        if (item.size() != 2) log::error("Incorrect formatting on %s", line.c_str());
        str item_name = lower(item.at(0).substr(ind * 2));
        str item_value = item.at(1).substr(1);
        bool component_found = false;
        
        for_<Component::ComponentType>([&](auto i){
            using C = std::variant_alternative_t<i.value, Component::ComponentType>;
            
            if (lower(str(type_name<C>())) == current_component) {
                C* component = scene_list.at(scene).getComponent<C>(eid);
                Reflection::apply(component, item_name, [item_value](auto *c){ assignFromString(*c, item_value); } );
                component_found = true;
            }
        });
        
        if (not component_found) log::error("You tried to load a component with an incorrect name, %s", current_component.c_str());
    }
    
    //: Get the component name
    if (state == LOAD_COMPONENT_NAME) {
        current_component = lower(line.substr(ind * 2, line.find(":") - ind * 2));
        
        if (add_components) {
            for_<Component::ComponentType>([&](auto i){
                using C = std::variant_alternative_t<i.value, Component::ComponentType>;
                
                if (lower(str(type_name<C>())) == current_component) {
                    scene_list.at(scene).addComponent<C>(eid);
                    state = LOAD_COMPONENT_BODY;
                }
            });
            
            if (state == LOAD_COMPONENT_NAME) log::error("The component '%s' is invalid, check the spelling", current_component.c_str());
        } else {
            state = LOAD_COMPONENT_BODY;
        }
    }
}

EntityID Serialization::loadEntity(str file, SceneID scene_id, str name) {
    EntityID id = -1;
    str entity_name;
    
    //: Load file
    str path = File::path("data/entities/" + file);
    std::ifstream f(path);
    LoadState state = LOAD_NAME;
    
    //: Line by line
    str s;
    while (std::getline(f, s)) {
        //: Indentation
        int indentation = getIndentation(s);
        if (indentation == -1) continue;
        
        //: Entity name
        if (state == LOAD_NAME) {
            auto l = split(s);
            if (l.size() != 2) log::error("You loaded an invalid entity, first line must be 'entity name', name can't contain spaces. %s", s.c_str());
            if (l.at(0) != "entity") log::error("You loaded an invalid entity, please make sure that the file starts with 'entity'");
            entity_name = name == "" ? l.at(1) : name;
            id = scene_list.at(scene_id).createEntity(entity_name);
            state = LOAD_FRONTMATTER;
            continue;
        }
        
        //: Entity frontmatter
        if (state == LOAD_FRONTMATTER) {
            //... (other frontmatter)
            
            if (s.at(0) == '-')
                state = LOAD_COMPONENT_NAME;
            continue;
        }
        
        //: Components
        loadComponents(s, state, scene_id, id, indentation);
    }
    
    return id;
}


SceneID Serialization::loadScene(str file) {
    SceneID scene_id = -1;
    EntityID current_eid = -1;
    bool add_components = true;
    
    //: Load file
    str path = File::path("data/scenes/" + file);
    std::ifstream f(path);
    LoadState state = LOAD_NAME;
    
    //: Line by line
    str s;
    while (std::getline(f, s)) {
        //: Indentation
        int indentation = getIndentation(s);
        if (indentation == -1) continue;
        
        //: Scene name
        if (state == LOAD_NAME) {
            auto l = split(s);
            if (l.size() != 2) log::error("You loaded an invalid scene, first line must be 'scene name', name can't contain spaces. %s", s.c_str());
            if (l.at(0) != "scene") log::error("You loaded an invalid scene, please make sure that the file starts with 'scene'");
            scene_id = registerScene(l.at(1));
            state = LOAD_FRONTMATTER;
            continue;
        }
        
        //: Scene frontmatter
        if (state == LOAD_FRONTMATTER) {
            //... (other frontmatter)
            
            if (s.at(0) == '-')
                state = LOAD_SCENE_ENTITY;
            continue;
        }
        
        //: Load compontents
        if (state == LOAD_COMPONENT_NAME or state == LOAD_COMPONENT_BODY) {
            if (indentation == 0) state = LOAD_SCENE_ENTITY;
            loadComponents(s, state, scene_id, current_eid, indentation, 1, add_components);
        }
        
        //: Load entity
        if (state == LOAD_SCENE_ENTITY) {
            auto l = split(s.substr(0, s.find(":")));
            if (l.at(0) != "entity") log::error("You loaded an invalid entity, please make sure that the file starts with 'entity'");
            
            if (l.size() == 2) { //: Entity from scratch
                current_eid = scene_list.at(scene_id).createEntity(l.at(1));
                state = LOAD_COMPONENT_NAME;
                add_components = true;
                continue;
            } else if (l.size() == 3) { //: Entity from template
                current_eid = loadEntity(l.at(2), scene_id, l.at(1));
                state = LOAD_COMPONENT_NAME;
                add_components = false;
                continue;
            } else {
                log::error("You loaded an invalid entity, the name must be either 'entity name:' or 'entity template name:'. %s", s.c_str());
            }
        }
    }
    
    return scene_id;
}
