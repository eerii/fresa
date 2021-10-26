//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_opengl_core.h"
#include "r_window.h"

#ifdef USE_OPENGL

using namespace Verse;
using namespace Graphics;

void OpenGL::createContext(WindowData &win) {
    context = SDL_GL_CreateContext(win.window);
    if (context == nullptr) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    if (SDL_GL_MakeCurrent(win.window, context)) {
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

#ifndef DISABLE_GUI
void OpenGL::initImGUI(WindowData &win) {
    imgui_context = ImGui::CreateContext();
    if (imgui_context == nullptr)
        log::error("Error creating ImGui context: ", SDL_GetError());
    ImPlot::CreateContext();
    io = ImGui::GetIO(); (void)io;
    if (not ImGui_ImplSDL2_InitForOpenGL(win.window, context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    glCheckError();
}
#endif

void OpenGL::createShaderData() {
    //Hard coded for now
    
    shaders["render"] = ShaderData("res/shaders/render.vertex", "res/shaders/render.frag");
    shaders["light"] = ShaderData("res/shaders/light.vertex", "res/shaders/light.frag");
    shaders["post"] = ShaderData("res/shaders/post.vertex", "res/shaders/post.frag");
    shaders["text"] = ShaderData("res/shaders/text.vertex", "res/shaders/text.frag");
    shaders["cam"] = ShaderData("res/shaders/cam.vertex", "res/shaders/cam.frag");
    shaders["window"] = ShaderData("res/shaders/window.vertex", "res/shaders/window.frag");
    shaders["noise"] = ShaderData("res/shaders/noise.vertex", "res/shaders/noise.frag");
    shaders["debug"] = ShaderData("res/shaders/debug.vertex", "res/shaders/debug.frag");
    
    shader_locations["render"] = {"mvp", "layer"};
    shader_locations["light"] = {"mvp", "light", "light_size", "light_distortion"};
    shader_locations["post"] = {"mvp", "use_light", "show_light", "tex", "light_tex"};
    shader_locations["text"] = {"mvp", "layer", "text_color", "same_color"};
    shader_locations["cam"] = {"mvp", "tex"};
    shader_locations["window"] = {"mvp", "tex", "is_background"};
    shader_locations["noise"] = {"mvp", "layer", "noise", "mask"};
    shader_locations["debug"] = {"mvp", "c"};
    
    for (auto &[key, s] : shaders) {
        s.compile(shader_locations[key]);
        log::graphics("Program (%s) ID: %d", key.c_str(), s.pid);
    }
    
    log::graphics("---");
    glCheckError();
}

#endif
