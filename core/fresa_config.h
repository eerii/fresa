//* fresa_config
//      engine wide global configuration using constexpr virtual functions, so all configuration is resolved at compile time
//      you may override the default values using a configuration file

//* example of a configuration file
//      #include "fresa_config.h"
//      namespace fresa
//      {
//          constexpr inline struct GameConfig : Config {
//              constexpr str_view name() const override { return "my name"; }
//          } config;
//      }
//: the default name of the configuration file is "config.h", but an alternative can be added using FRESA_CONFIG_FILE
#pragma once

#include "fresa_types.h"

namespace fresa
{
    struct Config {
        //: name
        constexpr str_view virtual name() const { return "fresa"; };
        //: version
        constexpr std::array<ui8, 3> virtual version() const { return {0, 4, 3}; };
    };
}

//* a configuration file can be specified using the directive FRESA_CONFIG_FILE, it defaults to "config.h"
#ifndef FRESA_CONFIG_FILE
#define FRESA_CONFIG_FILE "config.h"
#endif

//* if present, include the configuration file and check that it defines a valid fresa::config
#if __has_include(FRESA_CONFIG_FILE) & !defined(FRESA_CONFIG)
    #define FRESA_CONFIG //: guard to avoid an infinite loop
    #include FRESA_CONFIG_FILE
    static_assert(std::derived_from<decltype(fresa::config), fresa::Config>, "the config file must define fresa::config and it must be a subclass of fresa::Config");
#else
namespace fresa
{
    constexpr inline Config config;
}
#endif