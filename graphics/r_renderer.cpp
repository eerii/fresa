//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include <glm/ext.hpp>

#include "r_renderer.h"
#include "r_opengl.h"
#include "r_shader.h"
#include "r_textures.h"
#include "r_palette.h"

#include "gui.h"
#include "time.h"
#include "stb_image.h"

#include "system_list.h"

//TODO: ABSTRACT RENDERER

using namespace Verse;
using namespace Graphics;

namespace {
    SDL_GLContext context;

    ui8 pid[3];

    ui32 fbo, fb_tex;
    ui32 vao;
    ui32 vbo[2];

    float vertices[] = {
         0.0,  1.0,  0.0,  1.0,
         1.0,  1.0,  1.0,  1.0,
         0.0,  0.0,  0.0,  0.0,
         1.0,  0.0,  1.0,  0.0,
    };

    float fb_vertices[] = {
         0.0,  1.0,  0.0,  0.0,
         1.0,  1.0,  1.0,  0.0,
         0.0,  0.0,  0.0,  1.0,
         1.0,  0.0,  1.0,  1.0,
    };

    Vec2 *camera_centre;
    glm::mat4 *mat_camera;

    glm::mat4 mat_proj;
    glm::mat4 fb_mat_proj, fb_mat_view;
    glm::mat4 fb_mat_model, fb_mat_model_bg;
    ui8 mat_loc, fb_mat_loc, mat_loc_3D;

    ui32 palette_tex;
    glm::mat4 dither_mat = glm::mat4(
    {
        0.0,   8.0,   2.0,   10.0,
        12.0,  4.0,   14.0,  6.0,
        3.0,   11.0,  1.0,   9.0,
        15.0,  7.0,   13.0,  5.0
    });

    Vec2 previous_window_size;
    Vec2 stretch_factor;
    float light_distortion;
}

void Graphics::Renderer::GL::create(Config &c, SDL_Window* window) {
    //CONTEXT
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    if (SDL_GL_MakeCurrent(window, context) != 0) {
        log::error("Error making OpenGL Context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    
    //EXTENSIONS
    #ifndef __APPLE__
    //Initialize Extensions
    if (!initGLExtensions()) {
        log::error("Couldn't init GL extensions!");
        SDL_Quit();
        exit(-1);
    }
    #endif
    
    //V-SYNC
    SDL_GL_SetSwapInterval(1);
    
    //INFORMATION
    log::graphics("---");
    log::graphics("Vendor:          %s", glGetString(GL_VENDOR));
    log::graphics("Renderer:        %s", glGetString(GL_RENDERER));
    log::graphics("OpenGL Version:  %s", glGetString(GL_VERSION));
    log::graphics("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log::graphics("---");
    
    //IMGUI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();
    
    //SHADERS
    pid[0] = Graphics::Shader::compileProgram("res/shaders/render.vertex", "res/shaders/render.frag");
    log::graphics("Program (Render) ID: %d", pid[0]);
    pid[1] = Graphics::Shader::compileProgram("res/shaders/post.vertex", "res/shaders/post.frag");
    log::graphics("Program (Post) ID: %d", pid[1]);
    pid[2] = Graphics::Shader::compileProgram("res/shaders/render3d.vertex", "res/shaders/render3d.frag");
    log::graphics("Program (Render 3D) ID: %d", pid[2]);
    log::graphics("---");
    
    //FRAMEBUFFER
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    glGenTextures(1, &fb_tex);
    glBindTexture(GL_TEXTURE_2D, fb_tex);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c.resolution.x, c.resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log::error("Error creating Game Framebuffer: %d", glGetError());
        SDL_Quit();
        exit(-1);
    }
    
    //GENERATE VERTEX
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //VALIDATE PROGRAMS
    for (ui8 p : pid) {
        Graphics::Shader::validateProgram(p);
    }
    
    //VBO
    glGenBuffers(2, vbo);
    
    //MATRIX LOCATIONS
    mat_proj = glm::ortho(0.0f, (float)(c.resolution.x), 0.0f, (float)(c.resolution.y));
    mat_loc = glGetUniformLocation(pid[0], "mvp");
    fb_mat_loc = glGetUniformLocation(pid[1], "mvp");
    mat_loc_3D = glGetUniformLocation(pid[2], "mvp");
    
    //BLEND ALPHA
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //PALETTE TEX
    Graphics::Texture::loadTexture("res/graphics/palette_multi.png", palette_tex);
    
    //DITHER
    dither_mat /= 16.0;
    
    //CATCH ERRORS
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Init: %d", e);
    }
}

ui32 Graphics::Renderer::GL::createTexture(ui8* tex, int w, int h) {
    ui32 tex_id;
    
    glGenTextures(1, &tex_id);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  
    return tex_id;
}

void Graphics::Renderer::GL::renderTexture(ui32 &tex_id, Rect &src, Rect &dst, ui16 frames, Config &c, bool flip) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(pid[0]);
    
    if (frames > 1) {
        vertices[4*0 + 2] = (float)src.pos.x / (float)(src.size.x * (frames + 1));
        vertices[4*1 + 2] = vertices[4*0 + 2] + (float)src.size.x / (float)(src.size.x * (frames + 1));
        vertices[4*2 + 2] = vertices[4*0 + 2];
        vertices[4*3 + 2] = vertices[4*1 + 2];
    }
    
    if (flip) {
        vertices[4*0 + 2] = vertices[4*1 + 2];
        vertices[4*1 + 2] = vertices[4*2 + 2];
        vertices[4*2 + 2] = vertices[4*0 + 2];
        vertices[4*3 + 2] = vertices[4*1 + 2];
    }    
    
    glm::mat4 mat_model = glm::mat4(1.0f);
    
    mat_model = glm::translate(mat_model, glm::vec3(stretch_factor.x * (dst.pos.x / c.render_scale),
                                                    stretch_factor.y * (dst.pos.y / c.render_scale), 0.0f));
    mat_model = glm::scale(mat_model, glm::vec3(stretch_factor.x * (dst.size.x / c.render_scale),
                                                stretch_factor.y * (dst.size.y / c.render_scale), 1.0f));
    
    glUniformMatrix4fv(mat_loc, 1, GL_FALSE, glm::value_ptr(mat_proj * *mat_camera * mat_model));
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glUseProgram(pid[0]);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    vertices[4*0 + 2] = 0.0;
    vertices[4*1 + 2] = 1.0;
    vertices[4*2 + 2] = 0.0;
    vertices[4*3 + 2] = 1.0;
}

void Graphics::Renderer::GL::prepareTilemap(Rect &dst, Config &c, std::array<float, 24> &vertices) {
    for (int i = 0; i < 6; i++) {
        vertices[4*i + 0] = (i % 2 == 1) ? stretch_factor.x * (dst.size.x / c.render_scale) : 0.0;
        vertices[4*i + 0] += stretch_factor.x * (dst.pos.x / c.render_scale);
        vertices[4*i + 1] = (i < 2 or i == 3) ? stretch_factor.y * (dst.size.y / c.render_scale) : 0.0;
        vertices[4*i + 1] += stretch_factor.y * (dst.pos.y / c.render_scale);
    }
}

void Graphics::Renderer::GL::renderTilemap(ui32 &tex_id, float *vertices, int size) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(pid[0]);
    
    glUniformMatrix4fv(mat_loc, 1, GL_FALSE, glm::value_ptr(mat_proj * *mat_camera));
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * size, vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glDrawArrays(GL_TRIANGLES, 0, 6 * size);
}

void Graphics::Renderer::GL::render3D(float *vertices, int size, Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(pid[2]);
    
    Rect dst = Rect(128, 90, 24, 24);
    
    glm::mat4 mat_model = glm::mat4(1.0f);
    mat_model = glm::translate(mat_model, glm::vec3(stretch_factor.x * (dst.pos.x / c.render_scale),
                                                    stretch_factor.y * (dst.pos.y / c.render_scale), 0.0f));
    mat_model = glm::rotate(mat_model, (float)sin(Time::current * 0.001) * 6.28f, glm::vec3(1.0f, 0.5f, 0.0f));
    mat_model = glm::scale(mat_model, glm::vec3(stretch_factor.x * (dst.size.x / c.render_scale),
                                                stretch_factor.y * (dst.size.y / c.render_scale), 1.0f));
    
    glUniformMatrix4fv(mat_loc_3D, 1, GL_FALSE, glm::value_ptr(mat_proj * mat_model));
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * size, vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_TRIANGLES, 0, 3 * size);
}

void Graphics::Renderer::GL::clear(Scene &scene, Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (previous_window_size.x != c.window_size.x or previous_window_size.y != c.window_size.y) {
        previous_window_size = c.window_size;
        
        stretch_factor = Vec2((c.resolution.x * c.render_scale)/c.window_size.x, (c.resolution.y * c.render_scale)/c.window_size.y);
        light_distortion = c.resolution.x / c.resolution.y;
        
        fb_mat_model_bg = glm::scale(glm::mat4(1.0f), glm::vec3((c.window_size.x), (c.window_size.y), 1.0f));
        fb_mat_model = glm::scale(glm::mat4(1.0f), glm::vec3((c.resolution.x * c.render_scale), (c.resolution.y * c.render_scale), 1.0f));
        fb_mat_view = glm::translate(glm::mat4(1.0f), glm::vec3(c.window_padding.x, c.window_padding.y, 0.0f));
        fb_mat_proj = glm::ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
        
        glViewport( 0, 0, c.window_size.x, c.window_size.y );
        
#ifdef TILEMAP
        System::Tilemap::init(scene, c);
#endif
    }
}

void Graphics::Renderer::GL::useCamera(glm::mat4 *mat, Vec2 *pos) {
    mat_camera = mat;
    camera_centre = pos;
}

void Graphics::Renderer::GL::render(Config &c) {
    //RENDER TO WINDOW
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(pid[1]);
    
    //PALETTE
    Graphics::Palette::render(c, palette_tex, pid[1]);
    
    //LIGTH
#ifdef LIGHT
    if (c.use_light) {
        std::vector<glm::vec4> light_sources = System::Light::getLight();
        Vec2 light_correction = Vec2(c.window_size.x / (c.window_size.x - (c.window_padding.x * 2)),
                                     c.window_size.y / (c.window_size.y - (c.window_padding.y * 2)));
        for (int i = 0; i < light_sources.size(); i++) {
            light_sources[i].x += (0.5 - (camera_centre->x / c.resolution.x)) * light_correction.x;
            light_sources[i].y += (0.5 - (camera_centre->y / c.resolution.y)) * light_correction.y;
        }
        glUniform4fv(glGetUniformLocation(pid[1], "light"), (int)(light_sources.size()), reinterpret_cast<GLfloat *>(light_sources.data()));
        glUniform1i(glGetUniformLocation(pid[1], "light_size"), (int)(light_sources.size()));
        glUniform1f(glGetUniformLocation(pid[1], "light_distortion"), light_distortion);
        glUniform1i(glGetUniformLocation(pid[1], "use_light"), true);
    } else {
        glUniform1i(glGetUniformLocation(pid[1], "use_light"), false);
    }
#endif
    
    //DITHER
    if (c.use_dithering) {
        glUniformMatrix4fv(glGetUniformLocation(pid[1], "dither_mat"), 1, GL_FALSE, glm::value_ptr(dither_mat));
        glUniform1i(glGetUniformLocation(pid[1], "use_dithering"), true);
    }
    else {
        glUniform1i(glGetUniformLocation(pid[1], "use_dithering"), false);
    }
    
    //FB TEXTURE
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb_tex);
    glUniform1i(glGetUniformLocation(pid[1], "tex"), 0);
    
    //VERTEX DATA
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0 ]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //BACKGROUND
    glUniform1i(glGetUniformLocation(pid[1], "is_background"), true);
    glUniformMatrix4fv(fb_mat_loc, 1, GL_FALSE, glm::value_ptr(fb_mat_proj * fb_mat_model_bg));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //LEVEL
    glUniform1i(glGetUniformLocation(pid[1], "is_background"), false);
    glUniformMatrix4fv(fb_mat_loc, 1, GL_FALSE, glm::value_ptr(fb_mat_proj * fb_mat_view * fb_mat_model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    
}

void Graphics::Renderer::GL::present(SDL_Window* window) {
    SDL_GL_SwapWindow(window);
}

void Graphics::Renderer::GL::destroy() {
    //OPENGL
    
    //IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    //SDL
    SDL_GL_DeleteContext(context);
}
