//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_shader.h"

#include <fstream>
#include <streambuf>

#include "log.h"
#include "r_opengl.h"

using namespace Verse;

#ifndef __APPLE__
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;

bool Graphics::Shader::initGLExtensions() {
    glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");

    return glCreateShader && glShaderSource && glCompileShader && glGetShaderiv &&
        glGetShaderInfoLog && glDeleteShader && glAttachShader && glCreateProgram &&
        glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog &&
        glUseProgram;
}
#endif

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
        id = 0;
    } else {
        log::graphics("Shader Compiled Correctly, ID: %d", id);
    }
    
    return id;
}

ui8 Graphics::Shader::compileProgram(str vertex_file, str fragment_file) {
    ui8 pid = 0;
    ui8 vertex_shader, fragment_shader;

    pid = glCreateProgram();

    //COMPILE VERTEX SHADER
    std::ifstream f(vertex_file.c_str());
    std::string source((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    vertex_shader = compileShader(source.c_str(), GL_VERTEX_SHADER);
    
    //COMPILE FRAGMENT SHADER
    f=std::ifstream(fragment_file.c_str());
    source=std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
    fragment_shader = compileShader(source.c_str(), GL_FRAGMENT_SHADER);

    //ATTACH TO PROGRAM
    if(vertex_shader && fragment_shader) {
        glAttachShader(pid, vertex_shader);
        glAttachShader(pid, fragment_shader);
        glLinkProgram(pid);
        
        int log_len;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
        if(log_len > 0) {
            char* log = (char*)malloc(log_len * sizeof(char));
            glGetProgramInfoLog(pid, log_len, &log_len, log);
            
            log::error("Program compile error: %s", log);
            free(log);
        }
    }
    
    //DELETE SHADERS
    if(vertex_shader)
        glDeleteShader(vertex_shader);
    if(fragment_shader)
        glDeleteShader(fragment_shader);
    
    return pid;
}

void Graphics::Shader::validateProgram(ui8 pid) {
    glValidateProgram(pid);
    
    int log_len;
    glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
    if(log_len > 0) {
        char* log = (char*)malloc(log_len * sizeof(char));
        glGetProgramInfoLog(pid, log_len, &log_len, log);
        
        log::error("Program validation error: %s", log);
        free(log);
    }
}
