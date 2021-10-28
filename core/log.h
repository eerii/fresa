//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

#include <iostream>

namespace Verse
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
            std::cout << "[ DEBUG ] " << __PRETTY_FUNCTION__ << std::endl;
        }
    
        template<typename T>
        void reflection(T &p_refl) {
            std::cout << p_refl << std::endl;
        }
    }
}
