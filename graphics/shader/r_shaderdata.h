//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>

namespace Verse::Graphics
{
    struct ShaderData {
        str vertex_file;
        str frag_file;
        
        ui8 gl_pid;
        std::map<str, ui8> locations;
    };
}
