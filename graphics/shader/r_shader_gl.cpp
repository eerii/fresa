//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_shader.h"

#include <fstream>
#include <streambuf>

#include "log.h"

using namespace Verse;
using namespace Graphics;

ui8 Shader::compileShaderGL(const char* source, ui32 shader_type) {
    //CREATE SHADER FROM SOURCE
    ui8 id = glCreateShader(shader_type);
    glShaderSource(id, 1, &source, NULL);
    //COMPILE SHADER
    glCompileShader(id);

    //ERROR HANDLING
    int shader_compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &shader_compiled);
    if(shader_compiled != GL_TRUE) {
        std::cerr << "Shader Compilation Error, ID: " << id << std::endl;
        
        int log_len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char *log = (char*)malloc(log_len);
            glGetShaderInfoLog(id, log_len, &log_len, log);
            
            std::cerr << "Shader compile log: " << log << std::endl;
            free(log);
        }
        glDeleteShader(id);
        return 0;
    }
    
    log::graphics("Shader Compiled Correctly, ID: %d", id);
    return id;
}

ui8 Shader::compileProgramGL(str vert_source, str frag_source) {
    ui8 pid = 0;
    ui8 vert_shader = 0;
    ui8 frag_shader = 0;

    pid = (ui8)glCreateProgram();
    if (pid == 0) {
        log::error("Program creation failed");
        return 0;
    }

    //COMPILE VERTEX SHADER
    if (vert_source.size() > 0)
        vert_shader = compileShaderGL(vert_source.c_str(), GL_VERTEX_SHADER);
    
    //COMPILE FRAGMENT SHADER
    if (frag_source.size() > 0)
        frag_shader = compileShaderGL(frag_source.c_str(), GL_FRAGMENT_SHADER);

    //ATTACH TO PROGRAM
    if (vert_source.size() > 0)
        glAttachShader(pid, vert_shader);
    if (frag_source.size() > 0)
        glAttachShader(pid, frag_shader);
    glLinkProgram(pid);
    
    int program_linked = GL_FALSE;
    glGetProgramiv(pid, GL_LINK_STATUS, &program_linked);
    if(program_linked != GL_TRUE) {
        std::cerr << "Program Compilation Error, ID: " << pid << std::endl;
        
        int log_len;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = (char*)malloc(log_len * sizeof(char));
            glGetProgramInfoLog(pid, log_len, &log_len, log);
            
            std::cerr << "Program compile log: " << log << std::endl;
            free(log);
        }
        glDeleteProgram(pid);
        return 0;
    }
    
    //DELETE SHADERS
    if(vert_source.size() > 0)
        glDeleteShader(vert_shader);
    if(frag_source.size() > 0)
        glDeleteShader(frag_shader);
    
    GLenum e(glGetError());
    while (e != GL_NO_ERROR) {
       log::error("OpenGL Error during Compile Shader Program: %d");
    }
    
    return pid;
}

void Shader::validate(const ShaderData &shader) {
    glValidateProgram(shader.pid);
    
    int program_valid = GL_FALSE;
    glGetProgramiv(shader.pid, GL_VALIDATE_STATUS, &program_valid);
    if(program_valid != GL_TRUE) {
        int log_len;
        glGetProgramiv(shader.pid, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = (char*)malloc(log_len * sizeof(char));
            glGetProgramInfoLog(shader.pid, log_len, &log_len, log);
            
            std::cout << "[ ERROR ]: Program compile log: " << log << std::endl;
            free(log);
        }
        glDeleteProgram(shader.pid);
        
        log::error("Program Validation Error, ID: %d", shader.pid);
        return;
    }
}

#endif
