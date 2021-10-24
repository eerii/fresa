//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_opengl_core.h"
#include "r_window.h"

#ifdef USE_OPENGL

using namespace Verse;
using namespace Graphics;

void OpenGL::createContext(Config &c) {
    context = SDL_GL_CreateContext(c.window);
    if (context == nullptr) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    if (SDL_GL_MakeCurrent(c.window, context)) {
        log::error("Error making OpenGL Context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    Window::updateVsync(c);
    glCheckError();
}

#ifndef DISABLE_GUI
void OpenGL::initImGUI(Config &c) {
    imgui_context = ImGui::CreateContext();
    if (imgui_context == nullptr)
        log::error("Error creating ImGui context: ", SDL_GetError());
    ImPlot::CreateContext();
    io = ImGui::GetIO(); (void)io;
    if (not ImGui_ImplSDL2_InitForOpenGL(c.window, context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    glCheckError();
}
#endif

#endif
