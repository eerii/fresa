//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "log.h"

#include <yaml-cpp/yaml.h>

namespace Verse::Serialization
{
    void loadYAML(str name, YAML::Node &file);
    void writeYAML(str name, YAML::Node &file);
    void destroyYAML(str name);
    void appendYAML(str name, str key, YAML::Node &file, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, YAML::Node &file, bool overwrite=false);
    void removeYAML(str name, str key);
    void removeYAML(str name, std::vector<str> key);
}
