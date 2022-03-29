//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#ifdef USE_OPENGL

#include "r_opengl_api.h"

#include "log.h"
#include "gui.h"
#include "f_time.h"
#include "config.h"

using namespace Fresa;
using namespace Graphics;

namespace {
    std::vector<std::function<void()>> deletion_queue;
}

//Initialization
//----------------------------------------

void Graphics::configureAPI() {
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

void Graphics::createAPI() {
    api.context = GL::createContext(win);
    
    for (auto &[id, s] : api.shaders)
        GL::createShaderDataGL(id);
    
    processRendererDescription();
    
    ui32 temp_vao = GL::createVertexArray(); //Needed for shader validation
    deletion_queue.push_back([temp_vao](){glDeleteVertexArrays(1, &temp_vao);});
    GL::validateShaderData(temp_vao);
    
    api.window_vertex_buffer = GL::createVertexBuffer(api, Vertices::window);
    api.scaled_window_uniform = GL::createBuffer(sizeof(UniformBufferObject), GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
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

    #ifdef USE_GLEW
    if (glewInit() != GLEW_OK)
        log::error("Error initializing GLEW");
    #endif
    
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
    SDL_GL_SetSwapInterval(win.vsync ? 1 : 0);
    
    return context;
}

ShaderData GL::createShaderDataGL(ShaderID shader) {
    //---Prepare shaders---
    //      In this step we are loading the shader "name" and creating all the important elements it needs
    //      This includes reading the shader SPIR-V code, performing reflection on it and converting it to compatible GLSL
    //      The final code is then used to create a GL program which contains the shader information and we can use for rendering
    ShaderData &data = api.shaders.at(shader);
    
    //: Options
    spirv_cross::CompilerGLSL::Options options;
    options.version = 410; //: Max supported version
    options.es = false; //: Desktop OpenGL
    options.enable_420pack_extension = false; //: Strip binding of uniforms and samplers+
    
    //: Override for web
    #ifdef __EMSCRIPTEN__
    options.version = 300;
    options.es = true; //: Mobile OpenGL
    options.fragment.default_float_precision = options.Highp; //: High float precision, requirement for web
    #endif
    
    //: Variables where the GLSL converted code will be saved
    str vert_source_glsl, frag_source_glsl;
    
    //---Vertex shader---
    if (data.code.vert.has_value()) {
        ShaderCompiler compiler = getShaderCompiler(data.code.vert.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers)
            data.uniforms[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        
        //: Options
        compiler.set_common_options(options);
        
        //: Compile to GLSL
        vert_source_glsl = compiler.compile();
    }
    glCheckError();
    
    //---Fragment shader---
    if (data.code.frag.has_value()) {
        ShaderCompiler compiler = getShaderCompiler(data.code.frag.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers)
            data.uniforms[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        
        //: Combined image samplers
        for (const auto &res : resources.sampled_images)
            data.images[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        for (const auto &res : resources.subpass_inputs)
            data.images[res.name] = compiler.get_decoration(res.id, spv::DecorationBinding);
        
        //: Options
        compiler.set_common_options(options);
        
        //: Compile to GLSL
        frag_source_glsl = compiler.compile();
    }
    glCheckError();
    
    //: Compile program
    ui32 pid = GL::compileProgram(vert_source_glsl, frag_source_glsl);
    data.pid = pid;
    
    log::graphics("Program (%s) ID: %d", shader.c_str(), data.pid);
    
    //: Set uniform bindings
    log::graphics("Uniform binding:");
    for (auto &[n, bind]: data.uniforms) {
        ui32 index = glGetUniformBlockIndex(data.pid, n.c_str());
        glUniformBlockBinding(data.pid, index, bind);
        log::graphics(" - Uniform %s : Index %d, Binding %d", n.c_str(), index, bind);
        bind = index;
    }
    glCheckError();
    
    //: Set image bindings
    log::graphics("Image binding:");
    for (auto &[n, bind] : data.images) {
        int tex_location = glGetUniformLocation(data.pid, n.c_str());
        if (tex_location == -1)
            log::error("The image sampler at binding %d in the shader %s is not valid, probably because it is not used.", bind, shader.c_str());
        glUseProgram(data.pid);
        glUniform1i(tex_location, bind);
        glUseProgram(0);
        log::graphics(" - Sampler %s : Location %d, Binding %d", n.c_str(), tex_location, bind);
    }
    glCheckError();
    
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
    glCheckError();
    
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
    glCheckError();
    
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
    glCheckError();
    
    return pid;
}

void GL::validateShaderData(ui32 vao_id) {
    //---Validate shaders---
    glBindVertexArray(vao_id);
    for (const auto &[key, s] : api.shaders) {
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

AttachmentID Graphics::registerAttachment(AttachmentType type, Vec2<> size) {
    static AttachmentID id = 0;
    while (api.attachments.find(id) != api.attachments.end())
        id++;
        
    api.attachments[id] = AttachmentData{};
    AttachmentData &attachment = api.attachments[id];
    
    attachment.tex = GL::createAttachmentTexture(size, type);
    attachment.type = type;
    attachment.size = size;
    
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
    glCheckError();
    
    return tex;
}

SubpassID Graphics::registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list) {
    static SubpassID id = 0;
    while (api.subpasses.find(id) != api.subpasses.end())
        id++;
    
    log::graphics("Registering subpass %d:", id);
    api.subpasses[id] = SubpassData{};
    SubpassData &subpass = api.subpasses[id];
    
    for (auto &a_id : attachment_list)
        Map::subpass_attachment.add(id, a_id);
    subpass.external_attachments = external_attachment_list;
    
    ui8 depth_attachment_count = 0;
    
    for (auto &a_id : attachment_list) {
        const AttachmentData &attachment = api.attachments.at(a_id);
        bool first_in_chain = true;
        
        //: Input
        if (attachment.type & ATTACHMENT_INPUT) {
            for (int i = (int)id - 1; i >= 0; i--) {
                SubpassData &previous = api.subpasses.at(i);
                if (previous.attachment_descriptions.count(a_id)) {
                    if (previous.attachment_descriptions.at(a_id) == ATTACHMENT_INPUT) {
                        log::error("Can't use an input attachment in more than 2 subpasses (origin and destination)");
                    } else {
                        first_in_chain = false;
                        subpass.attachment_descriptions[a_id] = ATTACHMENT_INPUT;
                        subpass.previous_subpass_dependencies[a_id] = (SubpassID)i;
                        log::graphics(" - Input attachment: %d (Depends on subpass %d)", a_id, i);
                        break;
                    }
                }
            }
        }
        
        if (first_in_chain) {
            //: Color
            if (attachment.type & ATTACHMENT_COLOR) {
                subpass.attachment_descriptions[a_id] = ATTACHMENT_COLOR;
                log::graphics(" - Color attachment: %d", a_id);
            }
                
            //: Depth
            if (attachment.type & ATTACHMENT_DEPTH) {
                subpass.attachment_descriptions[a_id] = ATTACHMENT_DEPTH;
                subpass.has_depth = true;
                if (depth_attachment_count++ > 0)
                    log::error("A subpass can contain at most 1 depth attachment");
                log::graphics(" - Depth attachment: %d", a_id);
            }
        }
    }
    
    //: Framebuffer
    subpass.framebuffer = GL::createFramebuffer(id);
    log::graphics(" - This subpass includes the framebuffer %d", subpass.framebuffer);
        
    return id;
}

RenderPassID Graphics::registerRenderPass(std::vector<SubpassID> subpasses) {
    log::debug("Render passes are not used in OpenGL");
    return 0;
}

ui32 GL::createFramebuffer(SubpassID subpass) {
    //---Framebuffer---
    //      A texture that you can draw to, useful for multi step shader pipelines
    //      It can have a color, depth or both attachments
    ui32 fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
    std::vector<AttachmentID> attachments{};
    for (auto &[a_id, type] : api.subpasses.at(subpass).attachment_descriptions)
        if (type != ATTACHMENT_INPUT)
            attachments.push_back(a_id);
    
    ui8 color_attachment_count = 0;
    for (auto &attachment : attachments) {
        const AttachmentData &data = api.attachments.at(attachment);
        
        if (data.type & ATTACHMENT_SWAPCHAIN) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fb);
            return 0;
        }
        
        if (data.type & ATTACHMENT_COLOR) {
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
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment_value, GL_TEXTURE_2D, data.tex, 0);
        }
            
        if (data.type & ATTACHMENT_DEPTH) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, data.tex, 0);
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

//----------------------------------------



//Images
//----------------------------------------

TextureID Graphics::registerTexture(Vec2<> size, Channels ch, ui8* pixels) {
    //---Create texture---
    static TextureID id = 0;
    do id++;
    while (api.texture_data.find(id) != api.texture_data.end() or id == no_texture);
    
    api.texture_data[id] = TextureData{};
    TextureData &texture = api.texture_data[id];
    
    //: Parameters
    texture.w = size.x;
    texture.h = size.y;
    texture.ch = (int)ch;
    
    //: Create
    glGenTextures(1, &texture.id_);
    
    glBindTexture(GL_TEXTURE_2D, texture.id_);
    
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

void Graphics::updateDrawDescriptorSets(const DrawDescription &draw) { }
void Graphics::addIndirectDrawCommand(DrawDescription &description) { }

//----------------------------------------



//Test
//----------------------------------------

void Graphics::render() {
    Clock::time_point time_before_draw = time();
    
    //: Clear
    for (const auto &[s_id, data] : api.subpasses) {
        glBindFramebuffer(GL_FRAMEBUFFER, data.framebuffer);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    for (const auto &[s_id, data] : api.subpasses) {
        //: Bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, data.framebuffer);
        
        //: Shaders
        std::vector<ShaderID> shaders = getAtoB<ShaderID>(s_id, Map::subpass_shader);
        
        for (const auto &shader_id : shaders) {
            #ifdef DEBUG
            if (shader_id.rfind("debug", 0) == 0) {
                if (api.render_attachment < 0)
                    continue;
                if (shader_id.find("_" + std::to_string(api.render_attachment)) == str::npos)
                    continue;
                glClearColor(0.f, 0.f, 0.f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            #endif
            
            ShaderData &shader = api.shaders.at(shader_id);
            
            //: Bind shader
            glUseProgram(shader.pid);
            glCheckError();
            
            //: Set viewport
            auto attachments = getAtoB_v(s_id, Map::subpass_attachment, api.attachments);
            if (attachments.size() > 0)
                glViewport(0, 0, attachments.begin()->second.size.x, attachments.begin()->second.size.y); // All subpass attachments have the same size
            
            //---Draw shaders---
            if (shader.is_draw) {
                if (not api.draw_queue.count(shader_id) and not api.draw_queue_instanced.count(shader_id))
                    continue;
                
                //---Instanced rendering queue---
                if (api.shaders.at(shader_id).is_instanced) {
                    auto queue_uniform = api.draw_queue_instanced.at(shader_id);
                    for (const auto &[uniform_id, queue_geometry] : queue_uniform) {
                        DrawUniformData &uniform = api.draw_uniform_data.at(uniform_id);
                        
                        //: Upload uniforms
                        for (auto &[name, binding] : shader.uniforms)
                            glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform.uniform_buffers.at(binding).id_);
                        
                        for (const auto &[geometry_id, queue_instance] : queue_geometry) {
                            GeometryBufferData &geometry = api.geometry_buffer_data.at(geometry_id);
                            
                            //: Bind VAO
                            glBindVertexArray(geometry.vao);
                            glBindBuffer(GL_ARRAY_BUFFER, geometry.vertex_buffer.id_);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.index_buffer.id_);
                            glCheckError();
                            
                            //: Index type
                            GLenum index_type = GL_UNSIGNED_SHORT;
                            if (geometry.index_bytes == 4) index_type = GL_UNSIGNED_INT;
                            else if (geometry.index_bytes != 2) log::error("Unsupported index byte size %d", geometry.index_bytes);
                            
                            for (const auto &[instance_id, description] : queue_instance) {
                                InstancedBufferData &instance = api.instanced_buffer_data.at(instance_id);
                                
                                glBindBuffer(GL_ARRAY_BUFFER, instance.instance_buffer.id_);
                                
                                //: Draw
                                glDrawElementsInstanced(GL_TRIANGLES, geometry.index_size, index_type, (void*)0, instance.instance_count);
                                glCheckError();
                            }
                        }
                    }
                }
                //---Regular rendering queue---
                else {
                    auto queue_buffer = api.draw_queue.at(shader_id);
                    for (const auto &[buffer, queue_tex] : queue_buffer) {
                        GeometryBufferData &geometry = api.geometry_buffer_data.at(buffer);
                        
                        //: Bind VAO
                        glBindVertexArray(geometry.vao);
                        
                        //: Bind vertex and index buffers
                        glBindBuffer(GL_ARRAY_BUFFER, geometry.vertex_buffer.id_);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.index_buffer.id_);
                        glCheckError();
                        
                        //: Index type
                        GLenum index_type = GL_UNSIGNED_SHORT;
                        if (geometry.index_bytes == 4) index_type = GL_UNSIGNED_INT;
                        else if (geometry.index_bytes != 2) log::error("Unsupported index byte size %d", geometry.index_bytes);
                        
                        for (const auto &[tex_id, queue_uniform] : queue_tex) {
                            //: Bind texture
                            if (tex_id != no_texture) {
                                TextureData tex = api.texture_data.at(tex_id);
                                if (shader.images.size() == 0)
                                    log::error("You are drawing a texture with a shader that does not support texture inputs");
                                glActiveTexture(GL_TEXTURE0 + shader.images.begin()->second);
                                glBindTexture(GL_TEXTURE_2D, (tex_id == no_texture) ? 0 : tex.id_);
                            }
                            
                            for (auto [uniform_id, description] : queue_uniform) {
                                DrawUniformData &uniform = api.draw_uniform_data.at(uniform_id);
                                
                                //: Upload uniforms
                                for (auto &[name, binding] : shader.uniforms)
                                    glBindBufferBase(GL_UNIFORM_BUFFER, binding, uniform.uniform_buffers.at(binding).id_);
                                
                                //: Draw
                                glDrawElements(GL_TRIANGLES, geometry.index_size, index_type, (void*)0);
                                glCheckError();
                            }
                        }
                    }
                }
            }
            
            //---Post shaders---
            else {
                #ifdef DEBUG
                if (shader_id.rfind("debug", 0) == 0) {
                    if (api.render_attachment < 0)
                        continue;
                    if (shader_id.find("_" + std::to_string(api.render_attachment)) == str::npos)
                        continue;
                    if (api.attachments.at(api.render_attachment).type & ATTACHMENT_DEPTH and
                        api.attachments.at(api.render_attachment).type & ATTACHMENT_MSAA) {
                        bool msaa_shader = shader_id.find("_msaa") != str::npos;
                        if (msaa_shader) //TODO: ADD SAMPLE CHECK
                            continue;
                    }
                }
                #endif
                
                //: Bind VAO
                glBindVertexArray(api.window_vertex_buffer.second);
                
                //: Bind vertex buffer
                glBindBuffer(GL_ARRAY_BUFFER, api.window_vertex_buffer.first.id_);
                
                //: Temporary - Scaled window UBO
                if (shader_id == "window")
                    glBindBufferBase(GL_UNIFORM_BUFFER, 0, api.scaled_window_uniform.id_);
                
                //: Get textures from attachments
                //  (These are different vectors since the Vulkan renderer handles them as different concepts, but in OpenGL they are treated the same)
                std::vector<ui32> textures{};
                
                for (auto &[a_id, type] : data.attachment_descriptions) //: Input attachments
                    if (type == ATTACHMENT_INPUT)
                        textures.push_back(api.attachments.at(a_id).tex);
                for (auto &a_id : data.external_attachments) //: External attachments
                    textures.push_back(api.attachments.at(a_id).tex);
                
                //: Binds images to the shader
                //      You need to provide the list of image attachments in the same order as the shader
                int i = 0;
                if (shader.images.size() != textures.size())
                    log::error("Mismatched attachment size between the shader and the attachments, review renderer_description. Make sure they are in the same order. Shader images: %d, Renderer description images: %d", shader.images.size(), textures.size());
                for (auto &[name, binding] : shader.images) {
                    glActiveTexture(GL_TEXTURE0 + binding);
                    glBindTexture(GL_TEXTURE_2D, textures.at(i++));
                }
                
                //: Draw
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glCheckError();
                
                //: Clear texture units
                for (int i = 0; i < textures.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
        }
    }
    
    Performance::render_draw_time = ms(time() - time_before_draw);
    
    //---Clear drawing queue---
    api.draw_queue.clear();
    api.draw_queue_instanced.clear();
    api.draw_descriptions.clear();
    
    //---Gui---
    IF_GUI(ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()));
}

void Graphics::present() {
    //---Present---
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SwapWindow(win.window);
}

//----------------------------------------



//Resize
//----------------------------------------

void Graphics::resize() {
    for (auto &[id, attachment] : api.attachments) {
        if (attachment.type & ATTACHMENT_WINDOW) {
            attachment.size = win.size;
            attachment.tex = GL::createAttachmentTexture(attachment.size, attachment.type);
        }
    }
    
    for (auto &[id, subpass] : api.subpasses) {
        if (subpass.framebuffer == 0)
            continue;
        
        glDeleteFramebuffers(1, &subpass.framebuffer);
        subpass.framebuffer = GL::createFramebuffer(id);
    }
    
    updateUniformBuffer(api.scaled_window_uniform, win.scaled_ubo);
    
    glCheckError();
}

//----------------------------------------



//Clean
//----------------------------------------

void Graphics::clean() {
    for (auto it = deletion_queue.rbegin(); it != deletion_queue.rend(); ++it)
        (*it)();
    deletion_queue.clear();
    
    log::graphics("Cleaned up OpenGL");
}

//----------------------------------------


#endif
