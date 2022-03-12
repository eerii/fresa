//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "load_obj.h"
#include "file.h"
#include <fstream>
#include <algorithm>

using namespace Fresa;
using namespace Graphics;

std::vector<VertexOBJ> Serialization::loadOBJ(str file) {
    //: Temporary objects
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec2> uvs{};
    std::vector<glm::vec3> normals{};
    
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
    }
    
    log::info("%d %d %d", positions.size(), uvs.size(), normals.size());
    
    std::vector<VertexOBJ> vertices;
    return vertices;
}
