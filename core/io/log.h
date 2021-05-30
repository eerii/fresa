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
    
        void vec2(Vec2 p_vector, str p_name = "");
        void vec2(Vec2f p_vector, str p_name = "");
    
        template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
        void num(T p_number, str p_name = "") {
            std::cout << "[NUMBER]: " << p_name << ((p_name == "") ? "" : ": ") << std::to_string(p_number) << std::endl;
        }
    
        //Disable if compiler doesn't support __PRETTY_FUNCTION__ (gcc and clang should)
        template <typename T>
        void debug_func(T&&) {
            std::cout << "[DEBUG]: " << __PRETTY_FUNCTION__ << std::endl;
        }
    }
}
