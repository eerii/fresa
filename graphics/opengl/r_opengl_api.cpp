//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"

#include "r_window.h"
#include "log.h"

using namespace Fresa;
using namespace Graphics;

namespace {
    std::vector<std::function<void()>> deletion_queue;
    
    const std::vector<VertexDataWindow> window_vertices = {
        {{-1.f, -1.f}}, {{-1.f, 1.f}},
        {{1.f, -1.f}}, {{1.f, 1.f}},
        {{1.f, -1.f}}, {{-1.f, 1.f}},
    };
    std::pair<BufferData, ui32> window_vertex_buffer;
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
    
    AttachmentID swapchain_attachment = GL::registerAttachment(gl.attachments, win.size, ATTACHMENT_COLOR_SWAPCHAIN);
    AttachmentID color_attachment = GL::registerAttachment(gl.attachments, win.size, ATTACHMENT_COLOR_INPUT);
    AttachmentID depth_attachment = GL::registerAttachment(gl.attachments, win.size, ATTACHMENT_DEPTH);
    
    SubpassID subpass_draw = GL::registerSubpass(gl.subpasses, gl.attachments, {color_attachment, depth_attachment});
    SubpassID subpass_post = GL::registerSubpass(gl.subpasses, gl.attachments, {swapchain_attachment, color_attachment});
    
    gl.shaders[SHADER_DRAW_COLOR] = GL::createShaderDataGL(shader_names.at(SHADER_DRAW_COLOR), subpass_draw);
    gl.shaders[SHADER_DRAW_TEX] = GL::createShaderDataGL(shader_names.at(SHADER_DRAW_TEX), subpass_draw);
    gl.shaders[SHADER_POST] = GL::createShaderDataGL(shader_names.at(SHADER_POST), subpass_post);
    
    ui32 temp_vao = GL::createVertexArray(); //Needed for shader validation
    deletion_queue.push_back([temp_vao](){glDeleteVertexArrays(1, &temp_vao);});
    GL::validateShaderData(temp_vao, gl.shaders);
    
    window_vertex_buffer = GL::createVertexBuffer(gl, window_vertices);
    
    return gl;
}

SDL_GLContext GL::createContext(const WindowData &win) {
    //---Context---
    //      Contrary to Vulkan, OpenGL does a lot of this under the hood. The context can be thought as a compendium of all those things.
    //      While in Vulkan we needed to initialize every single part of the graphics pipeline, in OpenGL we just create a context.
    SDL_GLContext context = SDL_GL_CreateContext(win.window);
    if (context == nullptr) {
        log::error("Error creating an OpenGL context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    
    //: Make the context active
    if (SDL_GL_MakeCurrent(win.window, context)) {
        log::error("Error making the OpenGL context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    glCheckError();
    
    //: Output some information
    log::graphics("");
    log::graphics("Vendor:          %s", glGetString(GL_VENDOR));
    log::graphics("Renderer:        %s", glGetString(GL_RENDERER));
    log::graphics("OpenGL version:  %s", glGetString(GL_VERSION));
    log::graphics("GLSL version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log::graphics("");
    glCheckError();
    
    //---Properties---
    //: Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    //: Depth test
    glEnable(GL_DEPTH_TEST);
    
    //: V-Sync
    SDL_GL_SetSwapInterval(1);
    
    return context;
}

ShaderData GL::createShaderDataGL(str name, SubpassID subpass) {
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
    
    log::graphics("Program (%s) ID: %d", name.c_str(), data.pid);
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
    
    //: Set framebuffer (equivalent to subpass in vulkan)
    data.subpass = subpass;
    
    deletion_queue.push_back([pid](){glDeleteProgram(pid);});
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
        std::cerr << "Shader compilation error, ID: " << id << std::endl;
        
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
    
    log::graphics("Shader compiled correctly, ID: %d", id);
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
        std::cerr << "Program compilation error, ID: " << pid << std::endl;
        
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
       log::error("OpenGL error while compiling shader program: %d");
    }
    
    return pid;
}

void GL::validateShaderData(ui32 vao_id, const std::map<Shaders, ShaderData> &shaders) {
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
            
            log::error("Program validation error, ID: %d", s.pid);
            return;
        }
    }
    glBindVertexArray(0);
}

//----------------------------------------



//Attachments
//----------------------------------------

AttachmentID GL::registerAttachment(std::map<AttachmentID, AttachmentData> &attachments, Vec2<> size, AttachmentType type) {
    static AttachmentID id = 0;
    while (attachments.find(id) != attachments.end())
        id++;
        
    attachments[id] = AttachmentData{};
    attachments[id].tex = GL::createAttachmentTexture(size, type);
    attachments[id].type = type;
    
    return id;
}

ui32 GL::createAttachmentTexture(Vec2<> size, AttachmentType type) {
    ui32 tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    if (type & ATTACHMENT_COLOR) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else if (type & ATTACHMENT_DEPTH) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    
    return tex;
}

SubpassID GL::registerSubpass(std::map<SubpassID, SubpassData> &subpasses, const std::map<AttachmentID, AttachmentData> &attachments,
                              std::vector<AttachmentID> list) {
    static SubpassID id = 0;
    while (subpasses.find(id) != subpasses.end())
        id++;
    
    log::graphics("Registering subpass %d:", id);
    subpasses[id] = SubpassData{};
    
    ui8 depth_attachment_count = 0;
    bool is_swapchain_subpass = false;
    
    for (auto &binding : list) {
        const AttachmentData &attachment = attachments.at(binding);
        
        //: Swapchain
        if (attachment.type & ATTACHMENT_SWAPCHAIN)
            is_swapchain_subpass = true;
        
        //: Input
        bool first_in_chain = true;
        if (attachment.type & ATTACHMENT_INPUT) {
            for (int i = id - 1; i >= 0; i--) {
                SubpassData &previous = subpasses.at(i);
                if (std::count(previous.input_attachments.begin(), previous.input_attachments.end(), attachment.tex)) {
                    log::error("Can't use an input attachment in more than 2 subpasses (origin and destination)");
                }
                if (std::count(previous.framebuffer_attachments.begin(), previous.framebuffer_attachments.end(), attachment.tex)) {
                    first_in_chain = false;
                    subpasses[id].input_attachments.push_back(binding);
                    log::graphics(" - Input attachment: %d (Depends on subpass %d)", binding, i);
                    break;
                }
            }
        }
        
        if (first_in_chain and not is_swapchain_subpass) {
            //: Color
            if (attachment.type & ATTACHMENT_COLOR) {
                subpasses[id].framebuffer_attachments.push_back(binding);
                log::graphics(" - Color attachment: %d", binding);
            }
                
            //: Depth
            if (attachment.type & ATTACHMENT_DEPTH) {
                subpasses[id].framebuffer_attachments.push_back(binding);
                subpasses[id].has_depth = true;
                depth_attachment_count++;
                log::graphics(" - Depth attachment: %d", binding);
            }
        }
    }
    
    if (depth_attachment_count > 1)
        log::error("A subpass can contain at most 1 depth attachment");
    
    subpasses[id].framebuffer = is_swapchain_subpass ? 0 : createFramebuffer(attachments, subpasses[id].framebuffer_attachments);
    log::graphics(" - This subpass includes the framebuffer %d", subpasses[id].framebuffer);
        
    return id;
}

ui32 GL::createFramebuffer(const std::map<AttachmentID, AttachmentData> &attachments, std::vector<AttachmentID> list) {
    //---Framebuffer---
    //      A texture that you can draw to, useful for multi step shader pipelines
    //      It can have a color, depth or both attachments
    ui32 fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
    ui8 color_attachment_count = 0;
    for (auto &attachment : list) {
        if (attachments.at(attachment).type & ATTACHMENT_SWAPCHAIN) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fb);
            return 0;
        }
        
        if (attachments.at(attachment).type & ATTACHMENT_COLOR) {
            GLenum color_attachment_value = [color_attachment_count](){
                if (color_attachment_count == 0) return GL_COLOR_ATTACHMENT0;
                if (color_attachment_count == 1) return GL_COLOR_ATTACHMENT1;
                if (color_attachment_count == 2) return GL_COLOR_ATTACHMENT2;
                if (color_attachment_count == 3) return GL_COLOR_ATTACHMENT3;
                if (color_attachment_count == 4) return GL_COLOR_ATTACHMENT4;
                if (color_attachment_count == 5) return GL_COLOR_ATTACHMENT5;
                if (color_attachment_count == 6) return GL_COLOR_ATTACHMENT6;
                if (color_attachment_count == 7) return GL_COLOR_ATTACHMENT7;
                log::error("No more color attachments allowed in one framebuffer");
                return 0;
            }();
            color_attachment_count++;
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment_value, GL_TEXTURE_2D, attachments.at(attachment).tex, 0);
        }
            
        if (attachments.at(attachment).type & ATTACHMENT_DEPTH) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachments.at(attachment).tex, 0);
        }
    }
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        log::error("Error creating OpenGL framebuffer: %d", glGetError());
    glCheckError();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return fb;
}

//----------------------------------------



//Buffers
//----------------------------------------

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

ui32 GL::createVertexArray() {
    //---Vertex array---
    //      Stores the layout of per vertex data that will be passed to the shader later
    ui32 vao_id;
    glGenVertexArrays(1, &vao_id);
    glCheckError();
    return vao_id;
}

BufferData GL::createIndexBuffer(const OpenGL &gl, const std::vector<ui16> &indices) {
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

DrawID API::registerDrawData(OpenGL &gl, DrawBufferID buffer, Shaders shader) {
    static DrawID id = 0;
    do id++;
    while (draw_data.find(id) != draw_data.end());
    
    draw_data[id] = DrawData{};
    
    draw_data[id].buffer_id = buffer;
    
    draw_data[id].uniform_buffers = { GL::createBuffer(sizeof(UniformBufferObject), GL_UNIFORM_BUFFER, GL_STREAM_DRAW) };
    
    draw_data[id].shader = shader;
    
    return id;
}

void API::updateDescriptorSets(const OpenGL &gl, const DrawData* draw) { }

//----------------------------------------



//Test
//----------------------------------------

void API::render(OpenGL &gl, WindowData &win) {
    //---Clear---
    for (const auto &[id, data] : gl.subpasses) {
        glBindFramebuffer(GL_FRAMEBUFFER, data.framebuffer);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    //---Draw shaders---
    for (const auto &[shader, buffer_queue] : API::draw_queue) {
        //: Bind framebuffer
        const SubpassData &subpass = gl.subpasses.at(gl.shaders.at(shader).subpass);
        glBindFramebuffer(GL_FRAMEBUFFER, subpass.framebuffer);
        
        //: Bind shader
        glUseProgram(gl.shaders.at(shader).pid);
        glCheckError();
        
        for (const auto &[buffer, tex_queue] : buffer_queue) {
            //: Bind VAO
            glBindVertexArray(buffer->vao);
            
            //: Bind vertex and index buffers
            glBindBuffer(GL_ARRAY_BUFFER, buffer->vertex_buffer.id_);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->index_buffer.id_);
            glCheckError();
            
            for (const auto &[tex, draw_queue] : tex_queue) {
                //...Bind texture
                if (tex == &no_texture)
                    glBindTexture(GL_TEXTURE_2D, 0);
                else
                    glBindTexture(GL_TEXTURE_2D, tex->id_);
                
                for (const auto &[data, model] : draw_queue) {
                    //...Uniforms (improve)
                    UniformBufferObject ubo{};
                    ubo.view = glm::lookAt(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    ubo.proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
                    ubo.model = model;
                    
                    //: Update uniforms
                    GL::updateUniformBuffer(data->uniform_buffers[0], &ubo);
                    
                    //: Upload uniforms
                    for (auto &[name, index] : gl.shaders[shader].uniforms) {
                        // TODO: Improve, add type reflection
                        if (name == "UniformBufferObject")
                            glBindBufferBase(GL_UNIFORM_BUFFER, index, data->uniform_buffers[0].id_);
                    }
                    
                    //: Draw
                    glDrawElements(GL_TRIANGLES, buffer->index_size, GL_UNSIGNED_SHORT, (void*)0);
                    glCheckError();
                }
            }
        }
    }
    
    //---Post shaders---
    for (const auto &[shader, data] : gl.shaders) {
        if (shader <= LAST_DRAW_SHADER)
            continue;
        
        //: Bind framebuffer
        const SubpassData &subpass = gl.subpasses.at(gl.shaders.at(shader).subpass);
        glBindFramebuffer(GL_FRAMEBUFFER, subpass.framebuffer);
        
        //: Bind shader
        glUseProgram(gl.shaders.at(shader).pid);
        glCheckError();
        
        //: Bind VAO
        glBindVertexArray(window_vertex_buffer.second);
        
        //: Bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, window_vertex_buffer.first.id_);
        
        //: Previous textures from attachments
        for (auto &binding : subpass.input_attachments) {
            glBindTexture(GL_TEXTURE_2D, gl.attachments.at(binding).tex);
        }
        
        //: Draw
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
        glCheckError();
    }
    
    glBindVertexArray(0);
    
    //---Present---
    SDL_GL_SetSwapInterval(0); //See if it is needed all frames
    SDL_GL_SwapWindow(win.window);
    
    //---Clear drawing queue---
    API::draw_queue.clear();
}

//----------------------------------------



//GUI
//----------------------------------------

void GL::GUI::initImGUI(OpenGL &gl, const WindowData &win) {
    #ifndef DISABLE_GUI
    gl.imgui_context = ImGui::CreateContext();
    if (gl.imgui_context == nullptr)
        log::error("Error creating ImGui context: ", SDL_GetError());
    //ImPlot::CreateContext();
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
    glViewport(0, 0, win.size.x, win.size.y);
    
    //TODO: Only resize attachments that depend on window size
    
    for (auto &[id, attachment] : gl.attachments)
        attachment.tex = GL::createAttachmentTexture(win.size, attachment.type);
    
    for (auto &[id, subpass] : gl.subpasses) {
        if (subpass.framebuffer == 0)
            continue;
        
        glDeleteFramebuffers(1, &subpass.framebuffer);
        subpass.framebuffer = GL::createFramebuffer(gl.attachments, subpass.framebuffer_attachments);
    }
    glCheckError();
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
