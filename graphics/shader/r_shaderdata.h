//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "r_shader.h"
#include <map>

#ifdef USE_OPENGL
#include "r_opengl.h"
#endif

namespace Verse::Graphics
{
    struct ShaderData {
        str vertex_file;
        str frag_file;
        
#ifdef USE_OPENGL
        ui8 pid;
        std::map<str, ui8> locations;
        
        ShaderData() {}
        ShaderData(str vertex, str frag) : vertex_file(vertex), frag_file(frag) {}
        
        void compile(std::vector<str> loc) {
            if (vertex_file.size() == 0 or frag_file.size() == 0)
                log::error("Tried to compile a shader without vertex or fragment file");
            
            pid = Shader::compileProgramGL(vertex_file, frag_file);
            
            for (str l : loc)
                locations[l] = glGetUniformLocation(pid, l.c_str());
            glCheckError();
        }
        
        void validate() {
            Shader::validateProgramGL(pid);
        }
#endif
        
    };
}
