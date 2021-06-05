//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_renderer.h"

#include "log.h"

#include "gui.h"
#ifndef DISABLE_GUI
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#endif

#include "r_opengl.h"
#include "r_shader.h"
#include "r_textures.h"
#include "r_palette.h"
#include "r_window.h"
#include "system_list.h"

#define CLIPPING_FAR -1000.0f
#define CLIPPING_NEAR 1000.0f

using namespace Verse;
using namespace Graphics;
using namespace glm;

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
        S_FIRE,
        S_DEBUG,
        S_LAST
    };
    ui8 shaders[S_LAST];

    //FRAMEBUFFERS
    ui32 fb_render, fb_post, fb_cam;
    ui32 tex_render, tex_post, tex_cam;
    ui32 depth_tex_render;

    //VERTICES
    enum VBOs {
        V_EMPTY = 0,
        V_TEX,
        V_TILE,
        V_FIRE,
        V_3D,
        V_POST,
        V_DRAW,
        V_TEST,
        V_DEBUG,
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

    float outlines[] = {
        0.01, 0.01,
        0.01, 0.99,
        1.0, 0.99,
        1.0, 0.01
    };

    //MATRICES
    mat4 proj_render, proj_post, proj_cam, proj_window;
    ui8 mvp[S_LAST];

    //MISC
    ui32 palette_tex;
    Vec2 previous_window_size;
    ui8 loc_layer, loc_layer_fire;
}



//CREATE
//-----------------------------------------
void Graphics::Renderer::create(Config &c, SDL_Window* window) {
    log::debug("Creating renderer");
    
    //CONTEXT
    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        log::error("Error creating OpenGL Context: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    if (SDL_GL_MakeCurrent(window, context)) {
        log::error("Error making OpenGL Context current: %s", SDL_GetError());
        SDL_Quit();
        exit(-1);
    }
    glCheckError();
    
    //V-SYNC
    Graphics::Window::updateVsync(c);
    glCheckError();
    
    //INFORMATION
    log::graphics("---");
    log::graphics("Vendor:          %s", glGetString(GL_VENDOR));
    log::graphics("Renderer:        %s", glGetString(GL_RENDERER));
    log::graphics("OpenGL Version:  %s", glGetString(GL_VERSION));
    log::graphics("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log::graphics("---");
    
#ifndef DISABLE_GUI
    //IMGUI
    ImGuiContext* imgui_context = ImGui::CreateContext();
    if (imgui_context == nullptr)
        log::error("Error creating ImGui context: ", SDL_GetError());
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (not ImGui_ImplSDL2_InitForOpenGL(window, context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    glCheckError();
#endif
    
    //SHADERS
    //-----------------------------------------
    shaders[S_RENDER2D] = Graphics::Shader::compileProgram("res/shaders/render.vertex", "res/shaders/render.frag");
    log::graphics("Program (Render) ID: %d", shaders[S_RENDER2D]);
    
    shaders[S_RENDER3D] = Graphics::Shader::compileProgram("res/shaders/render3d.vertex", "res/shaders/render3d.frag");
    log::graphics("Program (Render 3D) ID: %d", shaders[S_RENDER3D]);
    
    shaders[S_POST] = Graphics::Shader::compileProgram("res/shaders/post.vertex", "res/shaders/post.frag");
    log::graphics("Program (Post) ID: %d", shaders[S_POST]);
    
    shaders[S_CAM] = Graphics::Shader::compileProgram("res/shaders/cam.vertex", "res/shaders/cam.frag");
    log::graphics("Program (Cam) ID: %d", shaders[S_CAM]);
    
    shaders[S_WINDOW] = Graphics::Shader::compileProgram("res/shaders/window.vertex", "res/shaders/window.frag");
    log::graphics("Program (Window) ID: %d", shaders[S_WINDOW]);
    
    shaders[S_FIRE] = Graphics::Shader::compileProgram("res/shaders/fire.vertex", "res/shaders/fire.frag");
    log::graphics("Program (Fire) ID: %d", shaders[S_FIRE]);
    
    shaders[S_DEBUG] = Graphics::Shader::compileProgram("res/shaders/debug.vertex", "res/shaders/debug.frag");
    log::graphics("Program (Debug) ID: %d", shaders[S_FIRE]);
    
    log::graphics("---");
    //-----------------------------------------
    glCheckError();
    
    
    //FRAMEBUFFERS
    //-----------------------------------------
    //Phase 1: Render to small texture
    Renderer::createDepthFramebuffer(c, fb_render, tex_render, depth_tex_render, c.resolution + Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH));
    
    //Phase 2: Postprocessing effects
    Renderer::createFramebuffer(c, fb_post, tex_post, c.resolution + Vec2(2*BORDER_WIDTH ,2*BORDER_WIDTH));
    
    //Phase 3: Smooth camera and size
    Renderer::createFramebuffer(c, fb_cam, tex_cam, c.resolution * c.render_scale);
    //-----------------------------------------
    glCheckError();
    
    
    //VERTICES
    //-----------------------------------------
    //VAOs
    glGenVertexArrays(V_LAST, vao);
    glBindVertexArray(vao[V_EMPTY]);
    
    //Validate Programs
    for (ui8 p : shaders) {
        if (p != shaders[S_EMPTY])
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
    
    //Phase 1c: Render fire
    glBindVertexArray(vao[V_FIRE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_FIRE]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 1d: Render 3D
    glBindVertexArray(vao[V_3D]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_3D]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 2: Post
    glBindVertexArray(vao[V_POST]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_POST]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 3 and 4: Draw the framebuffers
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Debug
    glBindVertexArray(vao[V_DEBUG]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DEBUG]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    //-----------------------------------------
    glCheckError();
    
    //MATRICES
    //-----------------------------------------
    mvp[S_RENDER2D] = glGetUniformLocation(shaders[S_RENDER2D], "mvp");
    mvp[S_RENDER3D] = glGetUniformLocation(shaders[S_RENDER3D], "mvp");
    mvp[S_FIRE] = glGetUniformLocation(shaders[S_FIRE], "mvp");
    mvp[S_POST] = glGetUniformLocation(shaders[S_POST], "mvp");
    mvp[S_CAM] = glGetUniformLocation(shaders[S_CAM], "mvp");
    mvp[S_WINDOW] = glGetUniformLocation(shaders[S_WINDOW], "mvp");
    mvp[S_DEBUG] = glGetUniformLocation(shaders[S_DEBUG], "mvp");
    
    proj_render = ortho(0.0f, (float)c.resolution.x + 2*BORDER_WIDTH, (float)c.resolution.y + 2*BORDER_WIDTH, 0.0f, CLIPPING_NEAR, CLIPPING_FAR);
    proj_post = ortho(0.0f, 1.0f, 0.0f, 1.0f);
    proj_cam = ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
    proj_window = ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
    //-----------------------------------------
    glCheckError();

    //LAYER LOCATION
    loc_layer = glGetUniformLocation(shaders[S_RENDER2D], "layer");
    loc_layer_fire = glGetUniformLocation(shaders[S_FIRE], "layer");
    glCheckError();
    
    //BLEND ALPHA
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCheckError();
    
    //PALETTE TEX
    Graphics::Texture::loadTexture("res/graphics/palette.png", palette_tex);
    glCheckError();
}
//-----------------------------------------



//PHASE 1: RENDER TO SMALL TEXTURE
//fb_render, tex_render (c.resolution) using shaders[S_RENDER2D] or shaders[S_RENDER3D]
//-----------------------------------------
void Graphics::Renderer::renderTexture(Config &c, ui32 &tex_id, glm::mat4 model, float* vertices, int layer) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_TEX]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(loc_layer, (float)layer);
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering texture)");
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}

void Graphics::Renderer::renderTilemap(Config &c, ui32 &tex_id, float *vertices, int size, int layer) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TILE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * size, vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(loc_layer, (float)layer);
    
    //Matrices
    mat4 model = translate(mat4(1.0f), vec3(-BORDER_WIDTH, -BORDER_WIDTH, 0.0f));
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering tilemap)");
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glDrawArrays(GL_TRIANGLES, 0, 6 * size);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}

void Graphics::Renderer::renderFire(Config &c, Rect2 &dst, ui32 &p_tex, ui32 &f_tex, int layer) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_FIRE]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_FIRE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_FIRE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(loc_layer_fire, (float)layer);
    
    //Matrices
    mat4 model = matModel2D(dst.pos() - Vec2(BORDER_WIDTH, BORDER_WIDTH), dst.size());
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering fire)");
    glUniformMatrix4fv(mvp[S_FIRE], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p_tex);
    glUniform1i(glGetUniformLocation(shaders[S_FIRE], "pnoise"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, f_tex);
    glUniform1i(glGetUniformLocation(shaders[S_FIRE], "flame"), 1);
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}

void Graphics::Renderer::render3D(Config &c, float *vertices, int size) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER3D]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_3D]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_3D]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * size, vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //TODO: MATRICES
    glUniformMatrix4fv(mvp[S_RENDER3D], 1, GL_FALSE, glm::value_ptr(mat4(1.0f)));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLES, 0, 3 * size);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}
//-----------------------------------------



//PHASE 2: POSTPROCESSING
//fb_post, tex_post (c.resolution) using shaders[S_POST]
//-----------------------------------------
void Graphics::Renderer::renderPost(Config &c) {
    //Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(shaders[S_POST]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_POST]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_POST]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Palette
    Graphics::Palette::render(c, palette_tex, shaders[S_POST]);
    
#ifdef LIGHT
    //Light
    if (c.use_light)
        System::Light::render(c, shaders[S_POST]);
    glUniform1i(glGetUniformLocation(shaders[S_POST], "use_light"), c.use_light);
#endif
    
    //Set texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_render);
    glUniform1i(glGetUniformLocation(shaders[S_POST], "tex"), 1);
    
    //Matrices
    glUniformMatrix4fv(mvp[S_POST], 1, GL_FALSE, glm::value_ptr(proj_post));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}
//-----------------------------------------



//PHASE 3: SMOOTH CAMERA AND RESIZE
//fb_cam, tex_cam (c.resolution * c.render_scale) using shaders[S_CAM]
//-----------------------------------------
void Graphics::Renderer::renderCam(Config &c) {
    //Render Target: fb_cam
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glUseProgram(shaders[S_CAM]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_post);
    glUniform1i(glGetUniformLocation(shaders[S_CAM], "tex"), 0);
    
    //Matrices
    mat4 model = matModel2D(Vec2(-c.render_scale*BORDER_WIDTH, -c.render_scale*BORDER_WIDTH), (c.resolution + Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH)) * c.render_scale);
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering extra camera)");
    glUniformMatrix4fv(mvp[S_CAM], 1, GL_FALSE, glm::value_ptr(proj_cam * c.active_camera->m_extra * model));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}
//-----------------------------------------



//PHASE 4: RENDER TO WINDOW
//directly to window (c.window_size) using shaders[S_WINDOW]
//-----------------------------------------
void Graphics::Renderer::renderWindow(Config &c) {
    //Render Target: Window (fb 0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaders[S_WINDOW]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Palette for background
    Graphics::Palette::render(c, palette_tex, shaders[S_WINDOW]);
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_cam);
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "tex"), 0);
    
    //Matrices
    mat4 model = matModel2D((c.window_size - c.resolution * c.render_scale) * 0.5f, c.resolution * c.render_scale);
    mat4 modelbg = matModel2D(Vec2(0,0), c.window_size);
    glCheckError();
    
    //Draw Background
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "is_background"), true);
    glUniformMatrix4fv(mvp[S_WINDOW], 1, GL_FALSE, glm::value_ptr(proj_window * modelbg));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //Draw Level
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "is_background"), false);
    glUniformMatrix4fv(mvp[S_WINDOW], 1, GL_FALSE, glm::value_ptr(proj_window * model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}
//-----------------------------------------




//OTHER RENDERERS
//-----------------------------------------
void Graphics::Renderer::renderTest(Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaders[S_CAM]);
    glCheckError();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, palette_tex);
    glUniform1i(glGetUniformLocation(shaders[S_CAM], "tex"), 0);
    glCheckError();
    
    glBindVertexArray(vao[V_TEST]);
    glCheckError();
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEST]);
    glCheckError();
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glCheckError();
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    mat4 model = matModel2D(Vec2(0,0), c.window_size);
    glCheckError();
    
    glUniformMatrix4fv(mvp[S_CAM], 1, GL_FALSE, glm::value_ptr(proj_window * model));
    glCheckError();
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glCheckError();
}

void Graphics::Renderer::renderDebugCollider(Config &c, Rect2 col, bool colliding) {
    //Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(shaders[S_DEBUG]);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_DEBUG]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DEBUG]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outlines), outlines, GL_STATIC_DRAW);
    glCheckError();
    
    //Color
    vec3 color = vec3(0.32f, 0.89f, 0.60f);
    if (colliding)
        color = vec3(0.98f, 0.27f, 0.42f);
    
    glUniform3f(glGetUniformLocation(shaders[S_DEBUG], "c"), color.r, color.g, color.b);
    
    //Matrices
    mat4 model = matModel2D(col.pos() - Vec2(BORDER_WIDTH, BORDER_WIDTH), col.size());
    glUniformMatrix4fv(mvp[S_DEBUG], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();
}
//-----------------------------------------



//PRESENT
//-----------------------------------------
void Graphics::Renderer::present(SDL_Window* window) {
    SDL_GL_SwapWindow(window);
}
//-----------------------------------------



//CLEAR
//-----------------------------------------
void Graphics::Renderer::clear(Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glCheckError();
    
    if (previous_window_size == c.window_size)
        return;
    
    previous_window_size = c.window_size;
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    
    glBindTexture(GL_TEXTURE_2D, tex_cam);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c.resolution.x * c.render_scale, c.resolution.y * c.render_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_cam, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log::error("Error creating Game Framebuffer: %d", glGetError());
        SDL_Quit();
        exit(-1);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCheckError();
    
    proj_cam = ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
    proj_window = ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
    
#ifdef TILEMAP
    System::Tilemap::init(c);
#endif
    glCheckError();
}
//-----------------------------------------



//DESTROY
//-----------------------------------------
void Graphics::Renderer::destroy() {
    //OPENGL
    
#ifndef DISABLE_GUI
    //IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
#endif
    
    //SDL
    SDL_GL_DeleteContext(context);
}
//-----------------------------------------



//HELPERS
//-----------------------------------------
void Graphics::Renderer::createFramebuffer(Config &c, ui32 &fb, ui32 &tex, Vec2 res) {
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.x, res.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log::error("Error creating Game Framebuffer: %d", glGetError());
        SDL_Quit();
        exit(-1);
    }
    glCheckError();
}

void Graphics::Renderer::createDepthFramebuffer(Config &c, ui32 &fb, ui32 &tex, ui32 &d_tex, Vec2 res) {
    Graphics::Renderer::createFramebuffer(c, fb, tex, res);
    
    glGenTextures(1, &d_tex);
    glBindTexture(GL_TEXTURE_2D, d_tex);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, res.x, res.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d_tex, 0);
    glCheckError();
}

ui32 Graphics::Renderer::createTexture(ui8* tex, int w, int h, bool rgba) {
    ui32 tex_id;
    
    glGenTextures(1, &tex_id);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    (rgba) ? glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex) :
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  
    return tex_id;
}

void Graphics::Renderer::prepareTilemap(Config &c, Rect2 &dst, std::array<float, 24> &vertices) {
    //TODO: CHANGE FOR MAT MULT
    for (int i = 0; i < 6; i++) {
        vertices[4*i + 0] = (i % 2 == 1) ? dst.w : 0.0;
        vertices[4*i + 0] += dst.x;
        vertices[4*i + 1] = (i < 2 or i == 3) ? dst.h : 0.0;
        vertices[4*i + 1] += dst.y;
    }
}
//-----------------------------------------



//MATRICES
//-----------------------------------------
mat4 Graphics::Renderer::matModel2D(Vec2 pos, Vec2 size, float rotation) {
    mat4 s = scale(mat4(1.0f), vec3(size.x, size.y, 1.0f));
    mat4 r = rotate(mat4(1.0f), rotation, vec3(0.0f, 0.0f, 1.0f));
    mat4 t = translate(mat4(1.0f), vec3(pos.x, pos.y, 0.0f));
    
    return t * r * s;
}
mat4 Graphics::Renderer::matModel2D(Rect2 rect, float rotation) {
    return Graphics::Renderer::matModel2D(rect.pos(), rect.size(), rotation);
}

//-----------------------------------------
