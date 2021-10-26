//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"

#include "r_window.h"
#include "r_shader.h"

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
    GL::Init::createFramebuffers(gl, win);
    
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
    
    /*gl.shader_locations["render"] = {"mvp", "layer"};
    gl.shader_locations["light"] = {"mvp", "light", "light_size", "light_distortion"};
    gl.shader_locations["post"] = {"mvp", "use_light", "show_light", "tex", "light_tex"};
    gl.shader_locations["text"] = {"mvp", "layer", "text_color", "same_color"};
    gl.shader_locations["cam"] = {"mvp", "tex"};
    gl.shader_locations["window"] = {"mvp", "tex", "is_background"};
    gl.shader_locations["noise"] = {"mvp", "layer", "noise", "mask"};
    gl.shader_locations["debug"] = {"mvp", "c"};*/
    
    gl.shader_locations["test_gl"] = {"ubo"};
    //OpenGL needs version 410 core instead of 450, and uniforms do not have layout binding
    
    for (auto &[key, loc] : gl.shader_locations) {
        str vertex = "res/shaders/test/" + key + ".vertex"; //Remove the test
        str frag = "res/shaders/test/" + key + ".frag";
        gl.shaders[key] = Shader::create(vertex, frag, loc);
        log::graphics("Program (%s) ID: %d", key.c_str(), gl.shaders[key].pid);
    }
    
    log::graphics("---");
    glCheckError();
}

void GL::Init::createFramebuffers(OpenGL &gl, WindowData &win) {
    gl.framebuffer = GL::Buffers::createFramebuffer(win.size, FRAMEBUFFER_COLOR_ATTACHMENT);
    log::graphics("Created OpenGL framebuffers");
}

//----------------------------------------



//BUFFERS
//----------------------------------------

BufferData GL::Buffers::createFramebuffer(Vec2<> size, FramebufferType type) {
    BufferData buffer;
    
    glGenFramebuffers(1, &buffer.buffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.buffer_id);
    
    if (type == FRAMEBUFFER_COLOR_ATTACHMENT or type == FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT) {
        buffer.color_texture_id = 0;
        glGenTextures(1, &buffer.color_texture_id.value());
        glBindTexture(GL_TEXTURE_2D, buffer.color_texture_id.value());
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.color_texture_id.value(), 0);
    }
    
    if (type == FRAMEBUFFER_DEPTH_ATTACHMENT or type == FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT) {
        buffer.depth_texture_id = 0;
        glGenTextures(1, &buffer.depth_texture_id.value());
        glBindTexture(GL_TEXTURE_2D, buffer.depth_texture_id.value());
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depth_texture_id.value(), 0);
    }
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        log::error("Error creating Game Framebuffer: %d", glGetError());
    glCheckError();
    
    return buffer;
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
