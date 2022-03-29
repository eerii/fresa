//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"
#include "reflection.h"

#include <iostream>

#define PREPARE_LOG(in, out) char out[log_length]; \
va_list ap; \
va_start(ap, in); \
vsnprintf(out, sizeof(char) * log_length, in.c_str(), ap); \
va_end(ap);

#ifdef _MSC_VER
#define LOG_LEVEL 0 //TODO: Add support for MSVC
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

/*
 LOG LEVEL:
 1 - Errors
 2 - Errors and Warnings
 3 - Errors, Warnings and Info
 4 - Errors, Warnings, Info and Debug
 */

namespace Fresa
{
    namespace { inline constexpr int log_length = 1024; }
    
    namespace log
    {
        inline void error(str s, ...) {
        #if LOG_LEVEL>=1
            PREPARE_LOG(s, msg);
            std::cout << "[ ERROR ] " << msg << std::endl;
            throw std::runtime_error(msg);
        #endif
        };
        
        inline void warn(str s, ...) {
        #if LOG_LEVEL>=2
            PREPARE_LOG(s, msg);
            std::cout << "[ WARN ] " << msg << std::endl;
        #endif
        };
        
        template<typename ...T, std::enable_if_t<(true && ... && std::is_arithmetic_v<T>), bool> = true>
        void info(T ...t) { //: Multi purposed info print
        #if LOG_LEVEL>=3
            std::cout << "[ INFO ]";
            ((std::cout << " " << t), ...);
            std::cout << std::endl;
        #endif
        }
        
        template<typename T, std::enable_if_t<is_vec2_v<T>, bool> = true>
        void info(T v) { //: Vec2 print
        #if LOG_LEVEL>=3
            std::cout << "[ INFO ] x: " << v.x << " y: " << v.y << std::endl;
        #endif
        }
        
        template<typename T, std::enable_if_t<is_rect2_v<T>, bool> = true>
        void info(T r) { //: Rect2 print
        #if LOG_LEVEL>=3
            std::cout << "[ INFO ] x: " << r.x << " y: " << r.y << " w: " << r.w << " h: " << r.h << std::endl;
        #endif
        }
        
        template<typename T, std::enable_if_t<is_vector_v<T>, bool> = true>
        void info(T &v) { //: std::vector print
        #if LOG_LEVEL>=3
            std::cout << "[ INFO ]";
            for (auto &x : v) std::cout << " " << x;
            std::cout << std::endl;
        #endif
        }
        
        inline void info(str s, ...) { //: Print formatted strings with arguments
        #if LOG_LEVEL>=3
            PREPARE_LOG(s, msg);
            std::cout << "[ INFO ] " << msg << std::endl;
        #endif
        };
        
        inline void graphics(str s, ...) {
        #if LOG_LEVEL>=4
            PREPARE_LOG(s, msg);
            std::cout << "[ GRAPHICS ] " << msg << std::endl;
        #endif
        };
        
        inline void debug(str s, ...) {
        #if LOG_LEVEL>=4
            PREPARE_LOG(s, msg);
            std::cout << "[ DEBUG ] " << msg << std::endl;
        #endif
        };
    
        template <typename T>
        void debug_func(T&&) {
        #if LOG_LEVEL>=4
            #if defined _MSC_VER
            std::cout << "[ DEBUG ] " << _MSC_VER << std::endl;
            #else
            std::cout << "[ DEBUG ] " << __PRETTY_FUNCTION__ << std::endl;
            #endif
        #endif
        }
    }
}
