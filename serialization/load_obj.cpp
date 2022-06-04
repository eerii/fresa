//: fresa by jose pazos perez, licensed under GPLv3
#include "load_obj.h"
#include "file.h"
#include <fstream>

using namespace Fresa;
using namespace Graphics;

Serialization::VerticesOBJ Serialization::loadOBJ(str file) {
    VerticesOBJ obj{};
    
    //: Temporary objects
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec2> uvs{};
    std::vector<glm::vec3> normals{};
    std::vector<std::array<int, 3>> indices{};
    
    //: Load file
    file = File::path("models/" + file + ".obj");
    std::ifstream f(file);
    
    //: Line by line
    str s;
    while (std::getline(f, s)) {
        if (s.size() == 0 or s.at(0) == '#') continue; //: Skip blank lines and coments
        
        //: Name
        if (s.rfind("o ", 0) == 0) {
            log::debug("Loading OBJ object '%s' from file %s", split(s).at(1).c_str(), file.c_str());
            continue;
        }
        
        //: Vertex position
        if (s.rfind("v ", 0) == 0) {
            auto v = split(s); v.erase(v.begin());
            if (v.size() != 3) log::error("Formatting error for a vertex position");
            positions.push_back(glm::vec3(std::stof(v.at(0)), std::stof(v.at(1)), std::stof(v.at(2))));
            continue;
        }
        
        //: UV texture coordinates
        if (s.rfind("vt ", 0) == 0) {
            auto v = split(s); v.erase(v.begin());
            if (v.size() != 2) log::error("Formatting error for a vertex texture coordinates");
            uvs.push_back(glm::vec2(std::stof(v.at(0)), std::stof(v.at(1))));
            continue;
        }
        
        //: Vertex normals
        if (s.rfind("vn ", 0) == 0) {
            auto v = split(s); v.erase(v.begin());
            if (v.size() != 3) log::error("Formatting error for a vertex normal");
            normals.push_back(glm::vec3(std::stof(v.at(0)), std::stof(v.at(1)), std::stof(v.at(2))));
            continue;
        }
        
        //: Indices
        if (s.rfind("f ", 0) == 0) {
            auto v = split(s); v.erase(v.begin()); //: f 1/2/3 2/3/1 3/2/1
            for (auto v_ : v) {
                auto i_ = split(v_, "/");
                std::array<int, 3> i = {std::stoi(i_.at(0))-1, i_.at(1) == "" ? -1 : std::stoi(i_.at(1))-1, std::stoi(i_.at(2))-1};
                
                auto it = std::find(indices.begin(), indices.end(), i);
                obj.indices.push_back((decltype(obj.indices)::value_type)std::distance(indices.begin(), it));
                
                if (it == indices.end()) {
                    obj.vertices.push_back(VertexOBJ{positions.at(i.at(0)), i.at(1) == -1 ? glm::vec3(0.0f) : uvs.at(i.at(1)), normals.at(i.at(2))});
                    indices.push_back(i);
                }
            }
        }
    }
    
    return obj;
}
