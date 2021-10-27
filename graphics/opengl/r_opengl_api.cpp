//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"

#include "r_window.h"
#include "r_shader.h"
#include "r_vertex.h"

#include <glm/ext.hpp>
#include "ftime.h"

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
    GL::Init::createVertexArrays(gl);
    GL::Init::validateShaderData(gl);
    GL::Init::createVertexBuffers(gl);
    GL::Init::configureVertexAttributes(gl);
    GL::Init::configureProperties();
    
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
    
    gl.shader_locations["test_gl"] = {"mvp"};
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

void GL::Init::createVertexArrays(OpenGL &gl) {
    gl.vao = GL::Buffers::createVertexArray();
    log::graphics("Created OpenGL vertex arrays");
}

void GL::Init::validateShaderData(OpenGL &gl) {
    glBindVertexArray(gl.vao.id_);
    for (auto &[key, s] : gl.shaders)
        Shader::validate(s);
    glBindVertexArray(0);
}

void GL::Init::createVertexBuffers(OpenGL &gl) {
    gl.vbo = GL::Buffers::createVertexBuffer(gl.vao);
    log::graphics("Created OpenGL vertex buffers");
}

void GL::Init::configureVertexAttributes(OpenGL &gl) {
    glBindVertexArray(gl.vao.id_);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo.id_);
    
    for (const auto &attr : gl.vao.attributes) {
        ui32 size = (ui32)attr.format; //Assuming float
        glVertexAttribPointer(attr.location, size, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void*>(attr.offset));
        glEnableVertexAttribArray(attr.location);
    }
    glCheckError();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    log::graphics("Configured OpenGL vertex attributes, number of attributes: %d", gl.vao.attributes.size());
}

void GL::Init::configureProperties() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glCheckError();
}

//----------------------------------------



//BUFFERS
//----------------------------------------

FramebufferData GL::Buffers::createFramebuffer(Vec2<> size, FramebufferType type) {
    FramebufferData fb;
    
    glGenFramebuffers(1, &fb.id_);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.id_);
    
    if (type == FRAMEBUFFER_COLOR_ATTACHMENT or type == FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT) {
        fb.color_texture = TextureData();
        glGenTextures(1, &fb.color_texture.value().id_);
        glBindTexture(GL_TEXTURE_2D, fb.color_texture.value().id_);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.color_texture.value().id_, 0);
    }
    
    if (type == FRAMEBUFFER_DEPTH_ATTACHMENT or type == FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT) {
        fb.depth_texture = TextureData();
        glGenTextures(1, &fb.depth_texture.value().id_);
        glBindTexture(GL_TEXTURE_2D, fb.depth_texture.value().id_);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depth_texture.value().id_, 0);
    }
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        log::error("Error creating Game Framebuffer: %d", glGetError());
    glCheckError();
    
    fb.type = type;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return fb;
}

VertexArrayData GL::Buffers::createVertexArray() {
    VertexArrayData vao;
    
    glGenVertexArrays(1, &vao.id_);
    vao.attributes = Vertex::getAttributeDescriptions();
    glCheckError();
    
    return vao;
}

BufferData GL::Buffers::createVertexBuffer(VertexArrayData &vao) {
    BufferData vbo;
    
    glBindVertexArray(vao.id_);
    glGenBuffers(1, &vbo.id_);
    glBindVertexArray(0);
    glCheckError();
    
    return vbo;
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



//TEST
//----------------------------------------

namespace {
    const std::array<VertexData, 6> vertices = {
        VertexData{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
        VertexData{{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        VertexData{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        
        VertexData{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        VertexData{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
        VertexData{{0.5f, 0.5f}, {0.7f, 0.2f, 0.7f}},
    };
}

void GL::renderTest(WindowData &win, RenderData &render) {
    //Clear
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //No framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //Test shader
    glUseProgram(render.api.shaders["test_gl"].pid);
    
    //Bind VAO and VBO
    glBindVertexArray(render.api.vao.id_);
    glBindBuffer(GL_ARRAY_BUFFER, render.api.vbo.id_);
    
    //Buffer data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    
    //Matrices (improve)
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.3f * std::sin(t * 1.570796f)));
    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
    glm::mat4 mvp = proj * view * model;
    
    glUniformMatrix4fv(render.api.shaders["test_gl"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));
    
    //Draw (add indexed)
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Present
    SDL_GL_SwapWindow(win.window);
}

//----------------------------------------


#endif
