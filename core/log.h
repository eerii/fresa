//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <iostream>

#include "dtypes.h"

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
        void num(int p_number, str p_name = "");
        void num(ui32 p_number, str p_name = "");
        void num(ui64 p_number, str p_name = "");
        void num(float p_number, str p_name = "");
        void num(double p_number, str p_name = "");
    }
}
