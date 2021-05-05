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

    void appendYAML(str name, str key, str value, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, str value, bool overwrite=false);
    void appendYAML(str name, str key, int num, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, int num, bool overwrite=false);
    void appendYAML(str name, str key, float num, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, float num, bool overwrite=false);
    void appendYAML(str name, str key, bool b, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, bool b, bool overwrite=false);
    void appendYAML(str name, str key, Vec2 vec, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, Vec2 vec, bool overwrite=false);
    void appendYAML(str name, str key, Rect rect, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, Rect rect, bool overwrite=false);

    void removeYAML(str name, str key);
    void removeYAML(str name, std::vector<str> key);
}

namespace YAML
{
    template<>
    struct convert<Verse::Vec2> {
      static Node encode(const Verse::Vec2& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
      }

      static bool decode(const Node& node, Verse::Vec2& rhs) {
        if(!node.IsSequence() || node.size() != 2) {
          return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
      }
    };

    template<>
    struct convert<Verse::Rect> {
      static Node encode(const Verse::Rect& rhs) {
        Node node;
        node.push_back(rhs.pos.x);
        node.push_back(rhs.pos.y);
        node.push_back(rhs.size.x);
        node.push_back(rhs.size.y);
        return node;
      }

      static bool decode(const Node& node, Verse::Rect& rhs) {
        if(!node.IsSequence() || node.size() != 4) {
          return false;
        }

        rhs.pos.x = node[0].as<float>();
        rhs.pos.y = node[1].as<float>();
        rhs.size.x = node[2].as<float>();
        rhs.size.y = node[3].as<float>();
        return true;
      }
    };
}
