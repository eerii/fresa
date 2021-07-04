//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "log.h"

#include <memory>

#define LOG_LEVEL 4
/*
 LOG LEVEL:
 1 - Errors
 2 - Errors and Warnings
 3 - Errors, Warnings, Info and Numbers
 4 - Errors, Warnings, Info, Numbers and Graphics
 5 - Debug
 */

using namespace Verse;

int lenght = 1024;

void log::info(str p_info, ...)
{
#if LOG_LEVEL>=3
    char msg[lenght];
    va_list ap;
    va_start(ap, p_info);
    vsnprintf(msg, sizeof(char) * lenght, p_info.c_str(), ap);
    va_end(ap);

    std::cout << "[INFO] " << msg << std::endl;
#endif
}

void log::warn(str p_info, ...)
{
#if LOG_LEVEL>=2
    char msg[lenght];
    va_list ap;
    va_start(ap, p_info);
    vsnprintf(msg, sizeof(char) * lenght, p_info.c_str(), ap);
    va_end(ap);

    std::cout << "[WARNING] " << msg << std::endl;
#endif
}

void log::error(str p_info, ...)
{
#if LOG_LEVEL>=1
    char msg[lenght];
    va_list ap;
    va_start(ap, p_info);
    vsnprintf(msg, sizeof(char) * lenght, p_info.c_str(), ap);
    va_end(ap);

    std::cerr << "[ERROR] " << msg << std::endl;
    
    throw std::runtime_error(msg);
#endif
}

void log::graphics(str p_info, ...)
{
#if LOG_LEVEL>=4
    char msg[lenght];
    va_list ap;
    va_start(ap, p_info);
    vsnprintf(msg, sizeof(char) * lenght, p_info.c_str(), ap);
    va_end(ap);

    std::cout << "[GRAPHICS] " << msg << std::endl;
#endif
}

void log::debug(str p_info, ...)
{
#if LOG_LEVEL>=5
    char msg[lenght];
    va_list ap;
    va_start(ap, p_info);
    vsnprintf(msg, sizeof(char) * lenght, p_info.c_str(), ap);
    va_end(ap);

    std::cout << "[DEBUG] " << msg << std::endl;
#endif
}

void log::vec2(Vec2 p_vector, str p_name) {
    std::cout << "[VECTOR] " << p_name << ((p_name == "") ? "" : " ") << "x: " << p_vector.x << " | y: " << p_vector.y << std::endl;
}
void log::vec2(Vec2f p_vector, str p_name) {
    std::cout << "[VECTOR] " << p_name << ((p_name == "") ? "" : " ") << "x: " << p_vector.x << " | y: " << p_vector.y << std::endl;
}
