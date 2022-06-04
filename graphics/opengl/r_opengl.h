//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#ifdef USE_OPENGL

#include "r_types.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define glCheckError() glCheckError_(__FILENAME__, __LINE__)

#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504

namespace Fresa::Graphics
{
    struct OpenGL {
        SDL_GLContext context;
        
        BufferData scaled_window_uniform;
        std::pair<BufferData, ui32> window_vertex_buffer;
    };

    [[maybe_unused]] static void glCheckError_(std::string file, int line) {
        GLenum e_code;
        while ((e_code = glGetError()) != GL_NO_ERROR) {
            std::string e_message;
            switch (e_code)
            {
                case GL_INVALID_ENUM:                  e_message = "INVALID_ENUM"; break;
                case GL_INVALID_VALUE:                 e_message = "INVALID_VALUE"; break;
                case GL_INVALID_OPERATION:             e_message = "INVALID_OPERATION"; break;
                case GL_STACK_OVERFLOW:                e_message = "STACK_OVERFLOW"; break;
                case GL_STACK_UNDERFLOW:               e_message = "STACK_UNDERFLOW"; break;
                case GL_OUT_OF_MEMORY:                 e_message = "OUT_OF_MEMORY"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: e_message = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << "[GL_ERROR]: " << e_message << " | " << file << " (" << line << ")" << std::endl;
        }
    }
}


#endif
