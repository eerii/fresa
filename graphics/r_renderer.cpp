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

using namespace Verse;
using namespace Graphics;

namespace {
    SDL_GLContext context;

    //SHADERS (Program IDs)
    enum Shaders {
        S_EMPTY = 0,
        S_RENDER2D,
        S_RENDER3D,
        S_POST,
        S_CAM,
        S_WINDOW,
        S_LAST
    };
    ui8 shaders[S_LAST];

    //FRAMEBUFFERS
    ui32 fb_render, fb_post, fb_cam;
    ui32 tex_render, tex_post, tex_cam;

    //VERTICES
    enum VBOs {
        V_EMPTY = 0,
        V_TEX,
        V_TILE,
        V_3D,
        V_DRAW,
        V_LAST
    };
    ui32 vao[V_LAST];
    ui32 vbo[V_LAST];

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

    //MATRICES
    Vec2 *camera_centre;
    glm::mat4 *m_camera;

    glm::mat4 mat_proj;
    glm::mat4 fb_mat_proj, fb_mat_view;
    glm::mat4 fb_mat_model, fb_mat_model_bg;

    ui8 mvp[S_LAST];

    //MISC
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



//CREATE
//-----------------------------------------
void Graphics::Renderer::create(Config &c, SDL_Window* window) {
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
    //-----------------------------------------
    shaders[S_RENDER2D] = Graphics::Shader::compileProgram("res/shaders/render.vertex", "res/shaders/render.frag");
    log::graphics("Program (Render) ID: %d", shaders[S_RENDER2D]);
    
    shaders[S_RENDER3D] = Graphics::Shader::compileProgram("res/shaders/render3d.vertex", "res/shaders/render3d.frag");
    log::graphics("Program (Render 3D) ID: %d", shaders[S_RENDER3D]);
    
    shaders[S_POST] = Graphics::Shader::compileProgram("res/shaders/post.vertex", "res/shaders/post.frag");
    log::graphics("Program (Post) ID: %d", shaders[S_POST]);
    
    shaders[S_CAM] = Graphics::Shader::compileProgram("res/shaders/post.vertex", "res/shaders/post.frag");
    log::graphics("Program (Cam) ID: %d", shaders[S_CAM]);
    
    shaders[S_WINDOW] = Graphics::Shader::compileProgram("res/shaders/window.vertex", "res/shaders/window.frag");
    log::graphics("Program (Window) ID: %d", shaders[S_WINDOW]);
    
    log::graphics("---");
    //-----------------------------------------
    
    
    //FRAMEBUFFERS
    //-----------------------------------------
    //Phase 1: Render to small texture
    Renderer::createFramebuffer(fb_render, tex_render, c.resolution, c);
    
    //Phase 2: Postprocessing effects
    Renderer::createFramebuffer(fb_post, tex_post, c.resolution, c);
    
    //Phase 3: Scaling and smooth camera movement
    Renderer::createFramebuffer(fb_cam, tex_cam, c.resolution * c.render_scale, c);
    //-----------------------------------------
    
    
    //VERTICES
    //-----------------------------------------
    //VAOs
    glGenVertexArrays(V_LAST, vao);
    glBindVertexArray(vao[V_EMPTY]);
    
    //Validate Programs
    for (ui8 p : shaders) {
        if(p != shaders[S_EMPTY])
            Graphics::Shader::validateProgram(p);
    }
    
    //VBOs
    glGenBuffers(V_LAST, vbo);
    
    //Attributes
    
    //Phase 1a: Render single textures
    glBindVertexArray(vao[V_TEX]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 1b: Render tilemap
    glBindVertexArray(vao[V_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TILE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 1c: Render 3D
    glBindVertexArray(vao[V_3D]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_3D]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 2, 3 and 4: Draw the framebuffers
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    //-----------------------------------------
    
    //MATRICES
    //-----------------------------------------
    mat_proj = glm::ortho(0.0f, (float)(c.resolution.x), 0.0f, (float)(c.resolution.y));
    
    mvp[S_RENDER2D] = glGetUniformLocation(shaders[S_RENDER2D], "mvp");
    mvp[S_RENDER3D] = glGetUniformLocation(shaders[S_RENDER3D], "mvp");
    mvp[S_POST] = glGetUniformLocation(shaders[S_POST], "mvp");
    mvp[S_CAM] = glGetUniformLocation(shaders[S_CAM], "mvp");
    mvp[S_WINDOW] = glGetUniformLocation(shaders[S_WINDOW], "mvp");
    //-----------------------------------------
    
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
//-----------------------------------------



//PHASE 1: RENDER TO SMALL TEXTURE
//fb_render, tex_render (c.resolution) using shaders[S_RENDER2D] or shaders[S_RENDER3D]
//-----------------------------------------
void Graphics::Renderer::renderTexture(ui32 &tex_id, Rect &src, Rect &dst, ui16 frames, Config &c, bool flip) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    
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
    
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(mat_proj * *m_camera * mat_model));
    
    glBindVertexArray(vao[V_TEX]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glUseProgram(shaders[S_RENDER2D]);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    vertices[4*0 + 2] = 0.0;
    vertices[4*1 + 2] = 1.0;
    vertices[4*2 + 2] = 0.0;
    vertices[4*3 + 2] = 1.0;
    
    glBindVertexArray(0);
}

void Graphics::Renderer::renderTilemap(ui32 &tex_id, float *vertices, int size) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(mat_proj * *m_camera));
    
    glBindVertexArray(vao[V_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TILE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * size, vertices, GL_STATIC_DRAW);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glDrawArrays(GL_TRIANGLES, 0, 6 * size);
    
    glBindVertexArray(0);
}

void Graphics::Renderer::render3D(float *vertices, int size, Config &c) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER3D]);
    
    Rect dst = Rect(128, 90, 24, 24);
    
    glm::mat4 mat_model = glm::mat4(1.0f);
    mat_model = glm::translate(mat_model, glm::vec3(stretch_factor.x * (dst.pos.x / c.render_scale),
                                                    stretch_factor.y * (dst.pos.y / c.render_scale), 0.0f));
    mat_model = glm::rotate(mat_model, (float)sin(Time::current * 0.001) * 6.28f, glm::vec3(1.0f, 0.5f, 0.0f));
    mat_model = glm::scale(mat_model, glm::vec3(stretch_factor.x * (dst.size.x / c.render_scale),
                                                stretch_factor.y * (dst.size.y / c.render_scale), 1.0f));
    
    glUniformMatrix4fv(mvp[S_RENDER3D], 1, GL_FALSE, glm::value_ptr(mat_proj * mat_model));
    
    glBindVertexArray(vao[V_3D]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_3D]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * size, vertices, GL_STATIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, 3 * size);
    
    glBindVertexArray(0);
}
//-----------------------------------------



//PHASE 2: POSTPROCESSING
//fb_post, text_post (c.resolution) using shaders[S_POST]
//-----------------------------------------
void Graphics::Renderer::renderPost(Config &c) {
    //Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(shaders[S_POST]);
    
    //Set texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_render);
    glUniform1i(glGetUniformLocation(shaders[S_POST], "tex"), 1);
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);
    
    //Matrices
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(c.resolution.x, c.resolution.y, 1.0f));
    glm::mat4 proj = glm::ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
    glUniformMatrix4fv(mvp[S_POST], 1, GL_FALSE, glm::value_ptr(proj * model));
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
//-----------------------------------------



//PHASE 3: SCALING AND SMOOTH CAMERA MOVEMENT
//fb_cam, tex_cam (c.render_scale * c.resolution) using shaders[S_CAM]
//-----------------------------------------
void Graphics::Renderer::renderCam(Config &c) {
    //Render Target: fb_cam
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glUseProgram(shaders[S_CAM]);
    
    //Set texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_post);
    glUniform1i(glGetUniformLocation(shaders[S_CAM], "tex"), 1);
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);
    
    //Matrices
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(c.resolution.x, c.resolution.y, 1.0f)); //TODO: CHANGE
    glm::mat4 proj = glm::ortho(0.0f, (float)(c.resolution.x), 0.0f, (float)(c.resolution.y));
    glUniformMatrix4fv(mvp[S_CAM], 1, GL_FALSE, glm::value_ptr(proj * model));
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
}
//-----------------------------------------

//PHASE 3: SCALING AND SMOOTH CAMERA MOVEMENT
//directly to window (c.window_size) using shaders[S_WINDOW]
//-----------------------------------------
void Graphics::Renderer::renderWindow(Config &c) {
    //Render Target: Window (fb 0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaders[S_WINDOW]);
    
    //TODO: CHANGE
    //-----------------------------------------
    //PALETTE
    Graphics::Palette::render(c, palette_tex, shaders[S_WINDOW]);
    
    //LIGTH
/*#ifdef LIGHT
    if (c.use_light) {
        std::vector<glm::vec4> light_sources = System::Light::getLight();
        Vec2 light_correction = Vec2(c.window_size.x / (c.window_size.x - (c.window_padding.x * 2)),
                                     c.window_size.y / (c.window_size.y - (c.window_padding.y * 2)));
        for (int i = 0; i < light_sources.size(); i++) {
            light_sources[i].x += (0.5 - (camera_centre->x / c.resolution.x)) * light_correction.x;
            light_sources[i].y += (0.5 - (camera_centre->y / c.resolution.y)) * light_correction.y;
        }
        glUniform4fv(glGetUniformLocation(shaders[S_WINDOW], "light"), (int)(light_sources.size()), reinterpret_cast<GLfloat *>(light_sources.data()));
        glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "light_size"), (int)(light_sources.size()));
        glUniform1f(glGetUniformLocation(shaders[S_WINDOW], "light_distortion"), light_distortion);
        glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_light"), true);
    } else {
        glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_light"), false);
    }
#endif*/
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_light"), false);
    
    //DITHER
    /*if (c.use_dithering) {
        glUniformMatrix4fv(glGetUniformLocation(shaders[S_WINDOW], "dither_mat"), 1, GL_FALSE, glm::value_ptr(dither_mat));
        glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_dithering"), true);
    }
    else {
        glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_dithering"), false);
    }*/
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "use_dithering"), false);
    //-----------------------------------------
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_cam);
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "tex"), 0);
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fb_vertices), fb_vertices, GL_STATIC_DRAW);
    
    //Draw Background
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "is_background"), true);
    glUniformMatrix4fv(mvp[S_WINDOW], 1, GL_FALSE, glm::value_ptr(fb_mat_proj * fb_mat_model_bg));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //Draw Level
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "is_background"), false);
    glUniformMatrix4fv(mvp[S_WINDOW], 1, GL_FALSE, glm::value_ptr(fb_mat_proj * fb_mat_view * fb_mat_model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
//-----------------------------------------

void Graphics::Renderer::present(SDL_Window* window) {
    SDL_GL_SwapWindow(window);
}

void Graphics::Renderer::clear(Scene &scene, Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
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

void Graphics::Renderer::destroy() {
    //OPENGL
    
    //IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    //SDL
    SDL_GL_DeleteContext(context);
}


//HELPERS
//-----------------------------------------
void Graphics::Renderer::bindCamera(glm::mat4 *mat, Vec2 *pos) {
    m_camera = mat;
    camera_centre = pos;
}

void Graphics::Renderer::createFramebuffer(ui32 &fb, ui32 &tex, Vec2 res, Config &c) {
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.x, res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log::error("Error creating Game Framebuffer: %d", glGetError());
        SDL_Quit();
        exit(-1);
    }
}

ui32 Graphics::Renderer::createTexture(ui8* tex, int w, int h) {
    ui32 tex_id;
    
    glGenTextures(1, &tex_id);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  
    return tex_id;
}

void Graphics::Renderer::prepareTilemap(Rect &dst, Config &c, std::array<float, 24> &vertices) {
    for (int i = 0; i < 6; i++) {
        vertices[4*i + 0] = (i % 2 == 1) ? stretch_factor.x * (dst.size.x / c.render_scale) : 0.0;
        vertices[4*i + 0] += stretch_factor.x * (dst.pos.x / c.render_scale);
        vertices[4*i + 1] = (i < 2 or i == 3) ? stretch_factor.y * (dst.size.y / c.render_scale) : 0.0;
        vertices[4*i + 1] += stretch_factor.y * (dst.pos.y / c.render_scale);
    }
}
//-----------------------------------------
