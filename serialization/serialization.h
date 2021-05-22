//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <yaml-cpp/yaml.h>

#include "config.h"

namespace Verse::Serialization
{
    void initYAML();

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
    void appendYAML(str name, str key, Vec2f vec, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, Vec2f vec, bool overwrite=false);
    void appendYAML(str name, str key, Rect2 rect, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, Rect2 rect, bool overwrite=false);
    void appendYAML(str name, str key, Rect2f rect, bool overwrite=false);
    void appendYAML(str name, std::vector<str> key, Rect2f rect, bool overwrite=false);

    void removeYAML(str name, str key);
    void removeYAML(str name, std::vector<str> key);

    void loadComponentsFromYAML(EntityID eid, str entity_name, YAML::Node &entity, Scene *s, Config &c);
    void loadScene(str name, Scene *s, Config &c);
    EntityID loadPlayer(Scene *s, Config &c);
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

        rhs.x = node[0].as<int>();
        rhs.y = node[1].as<int>();
        return true;
      }
    };

    template<>
    struct convert<Verse::Vec2f> {
      static Node encode(const Verse::Vec2f& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
      }

      static bool decode(const Node& node, Verse::Vec2f& rhs) {
        if(!node.IsSequence() || node.size() != 2) {
          return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
      }
    };

    template<>
    struct convert<Verse::Rect2> {
      static Node encode(const Verse::Rect2& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.w);
        node.push_back(rhs.h);
        return node;
      }

      static bool decode(const Node& node, Verse::Rect2& rhs) {
        if(!node.IsSequence() || node.size() != 4) {
          return false;
        }

        rhs.x = node[0].as<int>();
        rhs.y = node[1].as<int>();
        rhs.w = node[2].as<int>();
        rhs.h = node[3].as<int>();
        return true;
      }
    };

    template<>
    struct convert<Verse::Rect2f> {
      static Node encode(const Verse::Rect2f& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.w);
        node.push_back(rhs.h);
        return node;
      }

      static bool decode(const Node& node, Verse::Rect2f& rhs) {
        if(!node.IsSequence() || node.size() != 4) {
          return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.w = node[2].as<float>();
        rhs.h = node[3].as<float>();
        return true;
      }
    };
}
