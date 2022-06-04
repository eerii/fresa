//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "scene.h"
#include "log.h"
#include <charconv>

namespace Fresa::Serialization
{
    enum LoadState {
        LOAD_NAME,
        LOAD_FRONTMATTER,
        LOAD_COMPONENT_NAME,
        LOAD_COMPONENT_BODY,
        LOAD_SCENE_ENTITY,
    };
    
    int getIndentation(const str &line);
    void loadComponents(const str &line, LoadState &state, SceneID scene, EntityID eid, int ind, int base_ind = 0, bool add_components = true);
    EntityID loadEntity(str file, SceneID scene_id, str name = "");
    SceneID loadScene(str file);
    
    template <typename T>
    void assignFromString(T &x, str s) {
        //: Strings
        if constexpr (std::is_same_v<T, str>)
            x = s;
        
        //: Integral
        if constexpr (std::is_integral_v<T>)
            std::from_chars(s.data(), s.data() + s.size(), x);
        
        //: Floating (from_chars is still not implemented for floating point values)
        if constexpr (std::is_same_v<T, float>)
            x = std::stof(s);
        if constexpr (std::is_same_v<T, double>)
            x = std::stod(s);
        if constexpr (std::is_same_v<T, long double>)
            x = std::stold(s);
        
        //: Vec2 (Format: [x, y])
        if constexpr (is_vec2_v<T>) {
            auto v = split(list_contents(s), ",");
            if (v.size() != 2) log::error("The Vec2 format is incorrect, %s", s.c_str());
            std::for_each(v.begin(), v.end(), trim);
            assignFromString(x.x, v.at(0));
            assignFromString(x.y, v.at(1));
        }
        
        //: Rect2
        if constexpr (is_rect2_v<T>) {
            auto v = split(list_contents(s), ",");
            if (v.size() != 4) log::error("The Rect2 format is incorrect, %s", s.c_str());
            std::for_each(v.begin(), v.end(), trim);
            assignFromString(x.x, v.at(0)); assignFromString(x.y, v.at(1));
            assignFromString(x.w, v.at(2)); assignFromString(x.h, v.at(3));
        }
        
        //: std::vector
        if constexpr (is_vector_v<T>) {
            auto v = split(list_contents(s), ",");
            x.resize(v.size());
            for (int i = 0; i < v.size(); i++) {
                trim(v.at(i));
                assignFromString(x.at(i), v.at(i));
            }
        }
    }
}
