//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"

#include "r_window.h"

#include <fstream>
#include <streambuf>

using namespace Fresa;
using namespace Graphics;

namespace {
    std::vector<std::function<void()>> deletion_queue;
}

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
    
    gl.context = GL::createContext(win);
    GL::GUI::initImGUI(gl, win);
    
    gl.shaders["test"] = GL::createShaderDataGL("test");
    gl.framebuffer = GL::createFramebuffer(win.size, FRAMEBUFFER_COLOR_ATTACHMENT);
    gl.vao = GL::createVertexArray<VertexData>();
    deletion_queue.push_back([&gl](){glDeleteVertexArrays(1, &gl.vao.id_);});
    
    GL::validateShaderData(gl.vao.id_, gl.shaders);
    
    return gl;
}

SDL_GLContext GL::createContext(const WindowData &win) {
    //---Context---
    //      Contrary to Vulkan, OpenGL does a lot of this under the hood. The context can be thought as a compendium of all those things.
    //      While in Vulkan we needed to initialize every single part of the graphics pipeline, in OpenGL we just create a context.
    SDL_GLContext context = SDL_GL_CreateContext(win.window);
    if (context == nullptr) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    
    //: Make the context active
    if (SDL_GL_MakeCurrent(win.window, context)) {
        log::error("Error making OpenGL Context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    glCheckError();
    
    //: Output some information
    log::graphics("---");
    log::graphics("Vendor:          %s", glGetString(GL_VENDOR));
    log::graphics("Renderer:        %s", glGetString(GL_RENDERER));
    log::graphics("OpenGL Version:  %s", glGetString(GL_VERSION));
    log::graphics("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log::graphics("---");
    glCheckError();
    
    //---Properties---
    //: Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    //Depth test
    glEnable(GL_DEPTH_TEST);
    
    return context;
}

ShaderData GL::createShaderDataGL(str name) {
    //---Prepare shaders---
    //      In this step we are loading the shader "name" and creating all the important elements it needs
    //      This includes reading the shader SPIR-V code, performing reflection on it and converting it to compatible GLSL
    //      The final code is then used to create a GL program which contains the shader information and we can use for rendering
    ShaderData data{};
    
    //: Shader data
    //      Returns a ShaderData object which contains all the locations as well as the SPIR-V code
    data = API::createShaderData(name);
    
    //: Options
    spirv_cross::CompilerGLSL::Options options;
    options.version = 410; //: Max supported version
    options.es = false; //: Desktop OpenGL
    options.enable_420pack_extension = false; //: Strip binding of uniforms and samplers
    
    //: Override for web
    #ifdef __EMSCRIPTEN__
    options.version = 300;
    options.es = true; //: Mobile OpenGL
    #endif
    
    //: Variables where the GLSL converted code will be saved
    str vert_source_glsl, frag_source_glsl;
    
    //---Vertex shader---
    if (data.code.vert.has_value()) {
        ShaderCompiler compiler = API::getShaderCompiler(data.code.vert.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers)
            data.uniforms[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        
        //: Options
        compiler.set_common_options(options);
        
        //: Compile to GLSL
        vert_source_glsl = compiler.compile();
    }
    
    //---Fragment shader---
    if (data.code.frag.has_value()) {
        ShaderCompiler compiler = API::getShaderCompiler(data.code.frag.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers)
            data.uniforms[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        
        //: Combined image samplers
        /*for (const auto &res : resources.sampled_images)
            str name = res.name;*/
        
        //: Add high precission float as a requirement for Web
        #ifdef __EMSCRIPTEN__
        options.Highp = true;
        #endif
        
        //: Options
        compiler.set_common_options(options);
        
        //: Compile to GLSL
        frag_source_glsl = compiler.compile();
    }
    
    //: Compile program
    ui32 pid = GL::compileProgram(vert_source_glsl, frag_source_glsl);
    data.pid = pid;
    log::graphics("Program (test) ID: %d", data.pid);
    glCheckError();
    
    //: Set uniform bindings
    log::graphics("Uniform binding:");
    for (auto &[name, temp]: data.uniforms) {
        ui32 index = glGetUniformBlockIndex(data.pid, name.c_str());
        glUniformBlockBinding(data.pid, index, temp);
        log::graphics(" - Uniform %s : Index %d, Binding %d", name.c_str(), index, temp);
        temp = index;
    }
    glCheckError();
    
    deletion_queue.push_back([pid](){glDeleteProgram(pid);});
    log::graphics("---");
    return data;
}

ui8 GL::compileShader(const char* source, ui32 shader_type) {
    //CREATE SHADER FROM SOURCE
    ui8 id = glCreateShader(shader_type);
    glShaderSource(id, 1, &source, NULL);
    //COMPILE SHADER
    glCompileShader(id);

    //ERROR HANDLING
    int shader_compiled = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &shader_compiled);
    if(shader_compiled != GL_TRUE) {
        std::cerr << "Shader Compilation Error, ID: " << id << std::endl;
        
        int log_len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char *log = (char*)malloc(log_len);
            glGetShaderInfoLog(id, log_len, &log_len, log);
            
            std::cerr << "Shader compile log: " << log << std::endl;
            free(log);
        }
        glDeleteShader(id);
        return 0;
    }
    
    log::graphics("Shader Compiled Correctly, ID: %d", id);
    return id;
}

ui8 GL::compileProgram(str vert_source, str frag_source) {
    ui8 pid = 0;
    ui8 vert_shader = 0;
    ui8 frag_shader = 0;

    pid = (ui8)glCreateProgram();
    if (pid == 0) {
        log::error("Program creation failed");
        return 0;
    }

    //COMPILE VERTEX SHADER
    if (vert_source.size() > 0)
        vert_shader = compileShader(vert_source.c_str(), GL_VERTEX_SHADER);
    
    //COMPILE FRAGMENT SHADER
    if (frag_source.size() > 0)
        frag_shader = compileShader(frag_source.c_str(), GL_FRAGMENT_SHADER);

    //ATTACH TO PROGRAM
    if (vert_source.size() > 0)
        glAttachShader(pid, vert_shader);
    if (frag_source.size() > 0)
        glAttachShader(pid, frag_shader);
    glLinkProgram(pid);
    
    int program_linked = GL_FALSE;
    glGetProgramiv(pid, GL_LINK_STATUS, &program_linked);
    if(program_linked != GL_TRUE) {
        std::cerr << "Program Compilation Error, ID: " << pid << std::endl;
        
        int log_len;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = (char*)malloc(log_len * sizeof(char));
            glGetProgramInfoLog(pid, log_len, &log_len, log);
            
            std::cerr << "Program compile log: " << log << std::endl;
            free(log);
        }
        glDeleteProgram(pid);
        return 0;
    }
    
    //DELETE SHADERS
    if(vert_source.size() > 0)
        glDeleteShader(vert_shader);
    if(frag_source.size() > 0)
        glDeleteShader(frag_shader);
    
    GLenum e(glGetError());
    while (e != GL_NO_ERROR) {
       log::error("OpenGL Error during Compile Shader Program: %d");
    }
    
    return pid;
}

void GL::validateShaderData(VAO vao_id, const std::map<str, ShaderData> &shaders) {
    //---Validate shaders---
    glBindVertexArray(vao_id);
    for (const auto &[key, s] : shaders) {
        glValidateProgram(s.pid);
        
        int program_valid = GL_FALSE;
        glGetProgramiv(s.pid, GL_VALIDATE_STATUS, &program_valid);
        if(program_valid != GL_TRUE) {
            int log_len;
            glGetProgramiv(s.pid, GL_INFO_LOG_LENGTH, &log_len);
            if (log_len > 0) {
                char* log = (char*)malloc(log_len * sizeof(char));
                glGetProgramInfoLog(s.pid, log_len, &log_len, log);
                
                std::cout << "[ ERROR ]: Program compile log: " << log << std::endl;
                free(log);
            }
            glDeleteProgram(s.pid);
            
            log::error("Program Validation Error, ID: %d", s.pid);
            return;
        }
    }
    glBindVertexArray(0);
}

//----------------------------------------



//Buffers
//----------------------------------------

FramebufferData GL::createFramebuffer(Vec2<> size, FramebufferType type) {
    //---Framebuffer---
    //      A texture that you can draw to, useful for multi step shader pipelines
    //      It can have a color, depth or both attachments
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

BufferData GL::createBuffer(size_t size, GLenum type, GLenum usage) {
    //---Buffer object---
    //      Base structure that holds data
    //      It can be extended into a vertex buffer, index buffer or uniform buffer
    //      In the latter case, extra parameters can be passed for memory allocation
    BufferData buffer;
    
    glGenBuffers(1, &buffer.id_);
    
    if (size > 0) {
        glBindBuffer(type, buffer.id_);
        glBufferData(type, size, nullptr, usage);
        glBindBuffer(type, 0);
    }
    
    glCheckError();
    
    deletion_queue.push_back([buffer](){glDeleteBuffers(1, &buffer.id_);});
    return buffer;
}

BufferData API::createVertexBuffer(const OpenGL &gl, const std::vector<Graphics::VertexData> &vertices) {
    //---Vertex buffer---
    //      It holds the vertices for the vertex shader to read, as well as the attribute specification
    //      It needs to be tied to the vertex array object (vao)
    BufferData buffer = GL::createBuffer();
    
    glBindVertexArray(gl.vao.id_);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id_);
    
    for (const auto &attr : gl.vao.attributes) {
        ui32 size = (ui32)attr.format; //Assuming float
        glVertexAttribPointer(attr.location, size, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void*>(attr.offset));
        glEnableVertexAttribArray(attr.location);
    }
    
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), vertices.data(), GL_STATIC_DRAW);
    
    glCheckError();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return buffer;
}

BufferData API::createIndexBuffer(const OpenGL &gl, const std::vector<ui16> &indices) {
    //---Index buffer---
    //      We are going to draw the mesh indexed, which means that vertex data is not repeated and we need a list of which vertices to draw
    BufferData buffer = GL::createBuffer();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id_);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui16), indices.data(), GL_STATIC_DRAW);
    
    glCheckError();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    return buffer;
}

//----------------------------------------



//Images
//----------------------------------------

TextureID API::registerTexture(const OpenGL &gl, Vec2<> size, Channels ch, ui8* pixels) {
    //---Create texture---
    static TextureID id = 0;
    do id++;
    while (texture_data.find(id) != texture_data.end());
    
    texture_data[id] = TextureData{};
    
    //: Parameters
    texture_data[id].w = size.x;
    texture_data[id].h = size.y;
    texture_data[id].ch = (int)ch;
    
    //: Create
    glGenTextures(1, &texture_data[id].id_);
    
    glBindTexture(GL_TEXTURE_2D, texture_data[id].id_);
    
    auto channels = [ch](){
        switch(ch) {
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
    
    glTexImage2D(GL_TEXTURE_2D, 0, channels, size.x, size.y, 0, channels, GL_UNSIGNED_BYTE, pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return id;
}

//----------------------------------------



//Draw
//----------------------------------------

DrawBufferID API::registerDrawBuffer(const OpenGL &gl, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices) {
    //TODO: Comment
    static DrawBufferID id = 0;
    do id++;
    while (draw_buffer_data.find(id) != draw_buffer_data.end());
    
    draw_buffer_data[id] = DrawBufferData{};
    
    draw_buffer_data[id].vertex_buffer = API::createVertexBuffer(gl, vertices);
    draw_buffer_data[id].index_buffer = API::createIndexBuffer(gl, indices);
    draw_buffer_data[id].index_size = (ui32)indices.size();
    
    return id;
}

DrawID API::registerDrawData(OpenGL &gl, DrawBufferID buffer) {
    static DrawID id = 0;
    do id++;
    while (draw_data.find(id) != draw_data.end());
    
    draw_data[id] = DrawData{};
    
    draw_data[id].buffer_id = buffer;
    
    draw_data[id].uniform_buffers = { GL::createBuffer(sizeof(UniformBufferObject), GL_UNIFORM_BUFFER, GL_STREAM_DRAW) };
    
    return id;
}

void API::updateDescriptorSets(const OpenGL &gl, const DrawData* draw) { }

//----------------------------------------



//Test
//----------------------------------------

void API::renderTest(OpenGL &gl, WindowData &win) {
    //---Clear---
    glClearColor(0.01f, 0.01f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //---Prepare for drawing---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(gl.shaders.at("test").pid);
    glBindVertexArray(gl.vao.id_); //TODO: The VAO might need to be per Draw Buffer, and binding vertex buffers later might not be necessary
    
    //---Draw---
    for (const auto &[tex, draw] : API::draw_queue_textures) {
        //...Todo bind texture
        for (const auto &item : draw) {
            //: Bind vertex and index buffers
            glBindBuffer(GL_ARRAY_BUFFER, item.buffer->vertex_buffer.id_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, item.buffer->index_buffer.id_);
            
            //...Uniforms (improve)
            UniformBufferObject ubo{};
            ubo.view = glm::lookAt(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
            ubo.model = item.model;
            
            //: Update uniforms
            GL::updateUniformBuffer(item.data->uniform_buffers[0], &ubo);
            
            //: Upload uniforms
            for (auto &[name, index] : gl.shaders["test"].uniforms) {
                // TODO: Improve, add type reflection
                if (name == "UniformBufferObject")
                    glBindBufferBase(GL_UNIFORM_BUFFER, index, item.data->uniform_buffers[0].id_);
            }
            
            //: Draw
            glDrawElements(GL_TRIANGLES, item.buffer->index_size, GL_UNSIGNED_SHORT, (void*)0);
        }
    }
    
    //---Present---
    SDL_GL_SetSwapInterval(0); //See if it is needed all frames
    SDL_GL_SwapWindow(win.window);
    
    //---Clear drawing queue---
    API::draw_queue_textures.clear();
}

//----------------------------------------



//GUI
//----------------------------------------

void GL::GUI::initImGUI(OpenGL &gl, const WindowData &win) {
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
    //TODO: WRITE THIS
}

//----------------------------------------



//Clean
//----------------------------------------

void API::clean(OpenGL &gl) {
    for (auto it = deletion_queue.rbegin(); it != deletion_queue.rend(); ++it)
        (*it)();
    deletion_queue.clear();
    
    log::graphics("Cleaned up OpenGL");
}

//----------------------------------------


#endif
