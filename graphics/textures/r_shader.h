//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//thank you so much to
//- augusto ruiz (https://github.com/AugustoRuiz/sdl2glsl)
//- the cherno (https://youtu.be/71BLZwRGUJE)
//for the help with this part c:

#pragma once

#include "dtypes.h"
#include "r_shaderdata.h"

namespace Verse::Graphics::Shader
{
    ui8 compileShader(const char* source, ui32 shader_type);
    ui8 compileProgram(str vertex_file, str fragment_file);
    void validateProgram(ui8 pid);
}
