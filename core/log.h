//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"

#include <iostream>

namespace Fresa
{
    namespace log
    {
        void info(str p_info, ...);
        void warn(str p_info, ...);
        void error(str p_info, ...);
        void graphics(str p_info, ...);
        void debug(str p_info, ...);
    
        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        void vec2(Vec2<T> p_vector, str p_name = "") {
            std::cout << "[ VECTOR ] " << p_name << ((p_name == "") ? "" : " ") << "x: " << p_vector.x << " | y: " << p_vector.y << std::endl;
        };
    
        template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        void num(T p_number, str p_name = "") {
            std::cout << "[ NUMBER ] " << p_name << ((p_name == "") ? "" : ": ") << std::to_string(p_number) << std::endl;
        }
    
        //Disable if compiler doesn't support __PRETTY_FUNCTION__ (gcc and clang should)
        template <typename T>
        void debug_func(T&&) {
            #if defined _MSC_VER
            std::cout << "[ DEBUG ] " << _MSC_VER << std::endl;
            #else
            std::cout << "[ DEBUG ] " << __PRETTY_FUNCTION__ << std::endl;
            #endif
        }
    
        template<typename T>
        void reflection(T &p_refl) {
            std::cout << p_refl << std::endl;
        }
    }
}
