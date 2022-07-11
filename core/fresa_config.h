//* fresa_config
//      engine wide global configuration uses constexpr virtual functions, so all configuration is resolved at compile time
//      runtime and debug configuration is a simple struct, can be overrided using the parent construct, they are modifyiable by code at runtime
//      you may override the default values using a configuration file

//* example of a configuration file
//      #include "fresa_config.h"
//      namespace fresa
//      {
//          constexpr inline struct _EngineConfig : EngineConfig {
//              constexpr str_view name() const override { return "my name"; }
//          } engine_config;
//      
//          inline RunConfig run_config{ .something = "value" };
//
//          #ifdef FRESA_DEBUG
//          inline DebugConfig debug_config{};
//          #endif
//      }
//: the default name of the configuration file is "config.h", but an alternative can be added using FRESA_CONFIG_FILE
#pragma once

#include "std_types.h"

namespace fresa
{
    //* engine config (compile time)
    struct EngineConfig {
        //: name
        constexpr str_view virtual name() const { return "fresa"; };
        //: version
        constexpr std::array<ui8, 3> virtual version() const { return {0, 4, 5}; };
        //: unit tests to run (comma separated list)
        constexpr str_view virtual run_tests() const { return ""; };
        //: log level (see tools/log.h for the list of levels)
        constexpr ui32 virtual log_level() const { return 0b0000111; };
    };

    //* run config (run time)
    struct RunConfig {

    };

    //* debug config (run time, only on debug builds)
    #ifdef FRESA_DEBUG
    struct DebugConfig {

    };
    #endif
}

//* a configuration file can be specified using the directive FRESA_CONFIG_FILE, it defaults to "config.h"
#ifndef FRESA_CONFIG_FILE
#define FRESA_CONFIG_FILE "config.h"
#endif

//* if present, include the configuration file and check that it defines a valid fresa::config
#if __has_include(FRESA_CONFIG_FILE) & !defined(FRESA_CONFIG)
    #define FRESA_CONFIG //: guard to avoid an infinite loop
    #include FRESA_CONFIG_FILE
    static_assert(std::derived_from<decltype(fresa::engine_config), fresa::EngineConfig>, "the config file must define fresa::engine_config and it must be fresa::EngineConfig or a subclass of it");
    static_assert(std::derived_from<decltype(fresa::config), fresa::RunConfig>, "the config file must define fresa::config and it must be fresa::RunConfig or a subclass of it");
    #ifdef FRESA_DEBUG
    static_assert(std::derived_from<decltype(fresa::debug_config), fresa::DebugConfig>, "the config file must define fresa::debug_config and it must be fresa::DebugConfig or a subclass of it");
    #endif
#else
//* if not, use the default values
namespace fresa
{
    constexpr inline EngineConfig engine_config;
    constexpr inline RunConfig config;
    #ifdef FRESA_DEBUG
    constexpr inline DebugConfig debug_config;
    #endif
}
#endif