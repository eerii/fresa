//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_shader.h"

#include <fstream>
#include <streambuf>

#include "log.h"
#include "r_opengl.h"

using namespace Verse;

ui8 Graphics::Shader::compileShader(const char* source, ui32 shaderType) {
    //CREATE SHADER FROM SOURCE
    ui8 id = glCreateShader(shaderType);
    glShaderSource(id, 1, &source, NULL);
    //COMPILE SHADER
    glCompileShader(id);

    //ERROR HANDLING
    int shaderCompiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &shaderCompiled);
    if(shaderCompiled != GL_TRUE) {
        log::error("Shader Compilation Error, ID: %d", id);
        
        int log_len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char *log = (char*)malloc(log_len);
            glGetShaderInfoLog(id, log_len, &log_len, log);
            
            log::error("Shader compile log: %s", log);
            free(log);
        }
        glDeleteShader(id);
        return 0;
    }
    
    log::graphics("Shader Compiled Correctly, ID: %d", id);
    return id;
}

ui8 Graphics::Shader::compileProgram(str vertex_file, str fragment_file) {
    ui8 pid = 0;
    ui8 vertex_shader, fragment_shader;

    pid = glCreateProgram();
    if (pid == 0) {
        log::error("Program creation failed");
        return 0;
    }

    //COMPILE VERTEX SHADER
    std::ifstream f(vertex_file.c_str());
    std::string source((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
#ifdef __EMSCRIPTEN__
    std::string version("410 core");
    source.replace(source.find(version), version.size(), "300 es");
#endif
    vertex_shader = compileShader(source.c_str(), GL_VERTEX_SHADER);
    
    //COMPILE FRAGMENT SHADER
    f=std::ifstream(fragment_file.c_str());
    source=std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
#ifdef __EMSCRIPTEN__
    source.replace(source.find(version), version.size(), "300 es\nprecision highp float;");
#endif
    fragment_shader = compileShader(source.c_str(), GL_FRAGMENT_SHADER);

    //ATTACH TO PROGRAM
    if(vertex_shader && fragment_shader) {
        glAttachShader(pid, vertex_shader);
        glAttachShader(pid, fragment_shader);
        glLinkProgram(pid);
        
        int program_linked = GL_FALSE;
        glGetProgramiv(pid, GL_LINK_STATUS, &program_linked);
        if(program_linked != GL_TRUE) {
            log::error("Program Compilation Error, ID: %d", pid);
            
            int log_len;
            glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
            if (log_len > 0) {
                char* log = (char*)malloc(log_len * sizeof(char));
                glGetProgramInfoLog(pid, log_len, &log_len, log);
                
                log::error("Program compile log: %s", log);
                free(log);
            }
            glDeleteProgram(pid);
            return 0;
        }
    }
    
    //DELETE SHADERS
    if(vertex_shader)
        glDeleteShader(vertex_shader);
    if(fragment_shader)
        glDeleteShader(fragment_shader);
    
    GLenum e(glGetError());
    while (e != GL_NO_ERROR) {
       log::error("OpenGL Error during Compile Shader Program: %d");
    }
    
    return pid;
}

void Graphics::Shader::validateProgram(ui8 pid) {
    glValidateProgram(pid);
    
    int program_valid = GL_FALSE;
    glGetProgramiv(pid, GL_VALIDATE_STATUS, &program_valid);
    if(program_valid != GL_TRUE) {
        log::error("Program Validation Error, ID: %d", pid);
        
        int log_len;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = (char*)malloc(log_len * sizeof(char));
            glGetProgramInfoLog(pid, log_len, &log_len, log);
            
            log::error("Program compile log: %s", log);
            free(log);
        }
        glDeleteProgram(pid);
        return;
    }
}

#endif
