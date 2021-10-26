//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"
#include "r_window.h"

using namespace Verse;
using namespace Graphics;

//INITIALIZATION
//----------------------------------------

void GL::config() {
    #ifdef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        #ifndef __APPLE__
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif
    #endif
}

OpenGL GL::create(WindowData &win) {
    OpenGL gl;
    
    GL::Init::createContext(gl, win);
    GL::Init::initImGUI(gl, win);
    GL::Init::createShaderData(gl);
    
    return gl;
}

void GL::Init::createContext(OpenGL &gl, WindowData &win) {
    gl.context = SDL_GL_CreateContext(win.window);
    if (gl.context == nullptr) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    if (SDL_GL_MakeCurrent(win.window, gl.context)) {
        log::error("Error making OpenGL Context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    Window::updateVsync(true);
    glCheckError();
    
    log::graphics("---");
    log::graphics("Vendor:          %s", glGetString(GL_VENDOR));
    log::graphics("Renderer:        %s", glGetString(GL_RENDERER));
    log::graphics("OpenGL Version:  %s", glGetString(GL_VERSION));
    log::graphics("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log::graphics("---");
    glCheckError();
}

void GL::Init::createShaderData(OpenGL &gl) {
    //Hard coded for now
    
    gl.shader_locations["render"] = {"mvp", "layer"};
    gl.shader_locations["light"] = {"mvp", "light", "light_size", "light_distortion"};
    gl.shader_locations["post"] = {"mvp", "use_light", "show_light", "tex", "light_tex"};
    gl.shader_locations["text"] = {"mvp", "layer", "text_color", "same_color"};
    gl.shader_locations["cam"] = {"mvp", "tex"};
    gl.shader_locations["window"] = {"mvp", "tex", "is_background"};
    gl.shader_locations["noise"] = {"mvp", "layer", "noise", "mask"};
    gl.shader_locations["debug"] = {"mvp", "c"};
    
    for (auto &[key, loc] : gl.shader_locations) {
        str vertex = "res/shaders/" + key + ".vertex";
        str frag = "res/shaders/" + key + ".frag";
        gl.shaders[key] = Shader::create(vertex, frag, loc);
        log::graphics("Program (%s) ID: %d", key.c_str(), gl.shaders[key].pid);
    }
    
    log::graphics("---");
    glCheckError();
}

//----------------------------------------



//GUI
//----------------------------------------

void GL::Init::initImGUI(OpenGL &gl, WindowData &win) {
    #ifndef DISABLE_GUI
    gl.imgui_context = ImGui::CreateContext();
    if (gl.imgui_context == nullptr)
        log::error("Error creating ImGui context: ", SDL_GetError());
    ImPlot::CreateContext();
    gl.io = ImGui::GetIO();
    if (not ImGui_ImplSDL2_InitForOpenGL(win.window, gl.context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    glCheckError();
    #endif
}

//----------------------------------------


#endif
