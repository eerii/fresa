//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "serialization.h"
#include "file.h"
#include "log.h"
#include <fstream>

using namespace Fresa;

enum LoadState {
    LOAD_NAME,
    LOAD_FRONTMATTER,
    LOAD_COMPONENT_NAME,
    LOAD_COMPONENT_BODY,
};

EntityID Serialization::loadEntity(str file, SceneID scene_id) {
    //: Load file
    str path = File::path("data/entities/" + file);
    std::ifstream f(path);
    LoadState state = LOAD_NAME;
    
    //: Saved variables
    Scene &scene = scene_list.at(scene_id);
    EntityID id = -1;
    str name;
    str current_component = "";
    
    //: Line by line
    str s;
    while (std::getline(f, s)) {
        //: Indentation
        int indentation = 0;
        for (auto &c : s) {
            if (c == ' ') indentation++;
            else break;
        }
        indentation /= 2;
        
        if (s.size() <= indentation) continue;
        
        //: Entity name
        if (state == LOAD_NAME) {
            name = s;
            id = scene.createEntity(name);
            state = LOAD_FRONTMATTER;
            continue;
        }
        
        //: Entity frontmatter
        if (state == LOAD_FRONTMATTER) {
            //... (other frontmatter)
            
            if (s == "---")
                state = LOAD_COMPONENT_NAME;
            continue;
        }
        
        //: Components
        if (state == LOAD_COMPONENT_NAME or state == LOAD_COMPONENT_BODY) {
            state = (s.at(0) == ' ') ? LOAD_COMPONENT_BODY : LOAD_COMPONENT_NAME;
        }
        
        if (state == LOAD_COMPONENT_BODY) {
            if (current_component == "") log::error("You are loading a component body with an invalid component name '%s'", current_component.c_str());
            
            if (indentation == 0) {
                state = LOAD_COMPONENT_NAME; //: Next component
            } else {
                std::vector<str> item = split(s, ":");
                if (item.size() != 2) log::error("Incorrect formatting on %s", s.c_str());
                str item_name = item.at(0).substr(indentation * 2);
                str item_value = item.at(1).substr(1);
                
                for_<Component::ComponentType>([&](auto i){
                    using C = std::variant_alternative_t<i.value, Component::ComponentType>;
                    
                    if (lower(str(type_name<C>())) == lower(current_component)) {
                        C* component = scene.getComponent<C>(id);
                        Reflection::forEach(*component, [&](auto &&x, ui8 level, const char* name){
                            if (name == item_name) assignFromString(x, item_value);
                        });
                    }
                });
            }
        }
        
        if (state == LOAD_COMPONENT_NAME) {
            current_component = s.substr(0, s.find(":"));
            
            for_<Component::ComponentType>([current_component, id, &scene, &state](auto i){
                using C = std::variant_alternative_t<i.value, Component::ComponentType>;
                
                if (lower(str(type_name<C>())) == lower(current_component)) {
                    scene.addComponent<C>(id);
                    state = LOAD_COMPONENT_BODY;
                }
            });
            
            if (state == LOAD_COMPONENT_NAME) log::error("The component '%s' is invalid, check the spelling", current_component.c_str());
        }
    }
    
    return id;
}
