//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "serialization.h"
#include <filesystem>
#include <iostream>
#include <fstream>

using namespace Verse;

void Serialization::loadYAML(str name, YAML::Node &file) {
    str path = "res/serialization/" + name + ".yaml";
    
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
    str path = "res/serialization/" + name + ".yaml";
    std::ofstream fout(path);
    fout << file;
}

void Serialization::destroyYAML(str name) {
    str path = "res/serialization/" + name + ".yaml";
    
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
                log::info("hi?");
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
