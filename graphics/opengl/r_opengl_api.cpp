//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"
#include "r_api.h"

#include "r_window.h"
#include "r_shader.h"

#include <glm/ext.hpp>
#include "ftime.h"

using namespace Verse;
using namespace Graphics;

//Initialization
//----------------------------------------

void API::configure() {
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

OpenGL API::create(WindowData &win) {
    OpenGL gl;
    
    GL::Init::createContext(gl, win);
    GL::GUI::initImGUI(gl, win);
    GL::Init::createShaderData(gl);
    GL::Init::createFramebuffers(gl, win);
    GL::Init::createVertexArrays(gl);
    GL::Init::validateShaderData(gl);
    GL::Init::createVertexBuffers(gl);
    GL::Init::createIndexBuffer(gl);
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
    gl.framebuffer = GL::createFramebuffer(win.size, FRAMEBUFFER_COLOR_ATTACHMENT);
    log::graphics("Created OpenGL framebuffers");
}

void GL::Init::createVertexArrays(OpenGL &gl) {
    gl.vao = GL::createVertexArray<VertexData>();
    log::graphics("Created OpenGL vertex arrays");
}

void GL::Init::validateShaderData(OpenGL &gl) {
    glBindVertexArray(gl.vao.id_);
    for (auto &[key, s] : gl.shaders)
        Shader::validate(s);
    glBindVertexArray(0);
}

void GL::Init::createVertexBuffers(OpenGL &gl) {
    gl.vbo = GL::createVertexBuffer(gl.vao);
    log::graphics("Created OpenGL vertex buffers");
}

void GL::Init::createIndexBuffer(OpenGL &gl) {
    gl.ibo = GL::createBuffer(gl.vao);
    log::graphics("Created OpenGL index buffer");
}

void GL::Init::configureProperties() {
    //Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    //Depth test
    glEnable(GL_DEPTH_TEST);
    
    glCheckError();
}

//----------------------------------------



//Buffers
//----------------------------------------

FramebufferData GL::createFramebuffer(Vec2<> size, FramebufferType type) {
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

BufferData GL::createBuffer(VertexArrayData &vao) {
    BufferData buffer;
    
    glBindVertexArray(vao.id_);
    glGenBuffers(1, &buffer.id_);
    glBindVertexArray(0);
    
    return buffer;
}

BufferData GL::createVertexBuffer(VertexArrayData &vao) {
    BufferData vbo = createBuffer(vao);
    
    glBindVertexArray(vao.id_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id_);
    
    for (const auto &attr : vao.attributes) {
        ui32 size = (ui32)attr.format; //Assuming float
        glVertexAttribPointer(attr.location, size, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void*>(attr.offset));
        glEnableVertexAttribArray(attr.location);
    }
    glCheckError();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return vbo;
}

//----------------------------------------



//Images
//----------------------------------------

void API::createTexture(OpenGL &gl, TextureData &tex, ui8* pixels) {
    glGenTextures(1, &tex.id_);
    
    glBindTexture(GL_TEXTURE_2D, tex.id_);
    
    auto channels = [tex](){
        switch(tex.ch) {
            case 1:
                #ifdef __EMSCRIPTEN__
                return GL_LUMINANCE;
                #else
                return GL_RED;
                #endif
            case 2:
                return GL_RG;
            case 3:
                return GL_RGB;
            default:
                return GL_RGBA;
        }
    }();
    
    glTexImage2D(GL_TEXTURE_2D, 0, channels, tex.w, tex.h, 0, channels, GL_UNSIGNED_BYTE, pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

//----------------------------------------



//Test
//----------------------------------------

namespace {
    const std::vector<VertexData> vertices = {
        {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}}, //Light
        {{1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}}, //Teal
        {{1.f, 1.f, -1.f}, {1.000f, 0.815f, 0.019f}}, //Yellow
        {{-1.f, 1.f, -1.f}, {0.988f, 0.521f, 0.113f}}, //Orange
        {{-1.f, -1.f, 1.f}, {0.925f, 0.254f, 0.345f}}, //Red
        {{1.f, -1.f, 1.f}, {0.925f, 0.235f, 0.647f}}, //Pink
        {{1.f, 1.f, 1.f}, {0.658f, 0.180f, 0.898f}}, //Purple
        {{-1.f, 1.f, 1.f}, {0.258f, 0.376f, 0.941f}}, //Blue
    };

    const std::vector<ui16> indices = {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1,
        5, 4, 6, 6, 4, 7,
    };
}

void API::renderTest(WindowData &win, RenderData &render) {
    //Clear
    glClearColor(0.01f, 0.01f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //No framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //Test shader
    glUseProgram(render.api.shaders["test_gl"].pid);
    
    //Bind VAO, VBO and IBO
    glBindVertexArray(render.api.vao.id_);
    glBindBuffer(GL_ARRAY_BUFFER, render.api.vbo.id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render.api.ibo.id_);
    
    //Buffer data
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_STATIC_DRAW); //This can be done in advance
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui16), indices.data(), GL_STATIC_DRAW);
    
    //Matrices (improve)
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    model = glm::rotate(model, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.3f * std::sin(t * 1.570796f)));
    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
    glm::mat4 mvp = proj * view * model;
    
    glUniformMatrix4fv(render.api.shaders["test_gl"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));
    
    //Draw (add indexed)
    glDrawElements(GL_TRIANGLES, (ui32)indices.size(), GL_UNSIGNED_SHORT, (void*)0);
    
    //Present
    SDL_GL_SetSwapInterval(0); //See if it is needed all frames
    SDL_GL_SwapWindow(win.window);
}

//----------------------------------------



//GUI
//----------------------------------------

void GL::GUI::initImGUI(OpenGL &gl, WindowData &win) {
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



//Resize
//----------------------------------------

void API::resize(OpenGL &gl, WindowData &win) {
    
}

//----------------------------------------



//Clean
//----------------------------------------

void API::clean(OpenGL &gl) {
    glDeleteBuffers(1, &gl.vbo.id_);
    glDeleteBuffers(1, &gl.ibo.id_);
    
    for(auto &[key, val] : gl.shaders) {
        glDeleteProgram(val.pid);
    }
    
    glDeleteVertexArrays(1, &gl.vao.id_);
    
    log::graphics("Cleaned up OpenGL");
}

//----------------------------------------


#endif
