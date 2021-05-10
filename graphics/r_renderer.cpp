//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

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
        V_FIRE,
        V_3D,
        V_POST,
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

    //MATRICES
    Vec2f *camera_centre;
    mat4 *m_camera, *m_e_camera;
    mat4 proj_render, proj_post, proj_cam, proj_window;
    ui8 mvp[S_LAST];

    //MISC
    ui32 palette_tex;
    Vec2 previous_window_size;
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
    
    shaders[S_CAM] = Graphics::Shader::compileProgram("res/shaders/cam.vertex", "res/shaders/cam.frag");
    log::graphics("Program (Cam) ID: %d", shaders[S_CAM]);
    
    shaders[S_WINDOW] = Graphics::Shader::compileProgram("res/shaders/window.vertex", "res/shaders/window.frag");
    log::graphics("Program (Window) ID: %d", shaders[S_WINDOW]);
    
    shaders[S_FIRE] = Graphics::Shader::compileProgram("res/shaders/lume.vertex", "res/shaders/lume.frag");
    log::graphics("Program (Fire) ID: %d", shaders[S_FIRE]);
    
    log::graphics("---");
    //-----------------------------------------
    
    
    //FRAMEBUFFERS
    //-----------------------------------------
    //Phase 1: Render to small texture
    Renderer::createFramebuffer(fb_render, tex_render, c.resolution + Vec2(2,2), c);
    
    //Phase 2: Postprocessing effects
    Renderer::createFramebuffer(fb_post, tex_post, c.resolution + Vec2(2,2), c);
    
    //Phase 3: Smooth camera and size
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
    
    glBindVertexArray(0);
    //-----------------------------------------
    
    //MATRICES
    //-----------------------------------------
    mvp[S_RENDER2D] = glGetUniformLocation(shaders[S_RENDER2D], "mvp");
    mvp[S_RENDER3D] = glGetUniformLocation(shaders[S_RENDER3D], "mvp");
    mvp[S_FIRE] = glGetUniformLocation(shaders[S_FIRE], "mvp");
    mvp[S_POST] = glGetUniformLocation(shaders[S_POST], "mvp");
    mvp[S_CAM] = glGetUniformLocation(shaders[S_CAM], "mvp");
    mvp[S_WINDOW] = glGetUniformLocation(shaders[S_WINDOW], "mvp");
    
    proj_render = ortho(0.0f, (float)c.resolution.x + 2, (float)c.resolution.y + 2, 0.0f);
    proj_post = ortho(0.0f, 1.0f, 0.0f, 1.0f);
    proj_cam = ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
    proj_window = ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
    //-----------------------------------------
    
    //BLEND ALPHA
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //PALETTE TEX
    Graphics::Texture::loadTexture("res/graphics/palette_multi.png", palette_tex);
    
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
void Graphics::Renderer::renderTexture(ui32 &tex_id, Rect2 &src, Rect2 &dst, ui16 frames, Config &c, bool flip) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    
    //Animations and flip
    //TODO: CHANGE TO MAT MULT
    if (frames > 1) {
        vertices[4*0 + 2] = (float)src.x / (float)(src.w * (frames + 1));
        vertices[4*1 + 2] = vertices[4*0 + 2] + (float)src.w / (float)(src.w * (frames + 1));
        vertices[4*2 + 2] = vertices[4*0 + 2];
        vertices[4*3 + 2] = vertices[4*1 + 2];
    }
    if (flip) {
        vertices[4*0 + 2] = vertices[4*1 + 2];
        vertices[4*1 + 2] = vertices[4*2 + 2];
        vertices[4*2 + 2] = vertices[4*0 + 2];
        vertices[4*3 + 2] = vertices[4*1 + 2];
    }
    
    //Vertices
    glBindVertexArray(vao[V_TEX]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //Matrices
    mat4 model = matModel2D(dst.pos() - Vec2(1,1), dst.size()); //Add the border
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(proj_render * *m_camera * model));
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //Return vertices to previous state
    vertices[4*0 + 2] = 0.0;
    vertices[4*1 + 2] = 1.0;
    vertices[4*2 + 2] = 0.0;
    vertices[4*3 + 2] = 1.0;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Rendering Textures: %d", e);
    }
}

void Graphics::Renderer::renderTilemap(ui32 &tex_id, float *vertices, int size, Config &c) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER2D]);
    
    //Vertices
    glBindVertexArray(vao[V_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TILE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * size, vertices, GL_STATIC_DRAW);
    
    //Matrices
    mat4 model = translate(mat4(1.0f), vec3(-1.0f, -1.0f, 0.0f));
    glUniformMatrix4fv(mvp[S_RENDER2D], 1, GL_FALSE, glm::value_ptr(proj_render * *m_camera * model));
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glDrawArrays(GL_TRIANGLES, 0, 6 * size);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Rendering Tilemaps: %d", e);
    }
}

void Graphics::Renderer::renderFire(Rect2 &dst, ui32 &p_tex, ui32 &f_tex) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_FIRE]);
    
    //Vertices
    glBindVertexArray(vao[V_FIRE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_FIRE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //Matrices
    mat4 model = matModel2D(dst.pos() - Vec2(1,1), dst.size());
    glUniformMatrix4fv(mvp[S_FIRE], 1, GL_FALSE, glm::value_ptr(proj_render * *m_camera * model));
    
    //Textures
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, p_tex);
    glUniform1i(glGetUniformLocation(shaders[S_FIRE], "pnoise"), 2);
    /*glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, w_tex);
    glUniform1i(glGetUniformLocation(shaders[S_FIRE], "wnoise"), 3);*/
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, f_tex);
    glUniform1i(glGetUniformLocation(shaders[S_FIRE], "flame"), 4);
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Rendering Fire: %d", e);
    }
}

void Graphics::Renderer::render3D(float *vertices, int size, Config &c) {
    //Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(shaders[S_RENDER3D]);
    
    //TODO: MATRICES
    glUniformMatrix4fv(mvp[S_RENDER3D], 1, GL_FALSE, glm::value_ptr(mat4(1.0f)));
    
    glBindVertexArray(vao[V_3D]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_3D]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * size, vertices, GL_STATIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, 3 * size);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Rendering 3D Objects: %d", e);
    }
}
//-----------------------------------------



//PHASE 2: POSTPROCESSING
//fb_post, tex_post (c.resolution) using shaders[S_POST]
//-----------------------------------------
void Graphics::Renderer::renderPost(Config &c) {
    //Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(shaders[S_POST]);
    
    //Palette
    Graphics::Palette::render(c, palette_tex, shaders[S_POST]);
    
#ifdef LIGHT
    //Light
    if (c.use_light) {
        std::vector<glm::vec4> light_sources = System::Light::getLight();
        for (int i = 0; i < light_sources.size(); i++) {
            light_sources[i].x += (0.5 - (camera_centre->x / c.resolution.x));
            light_sources[i].y -= (0.5 - (camera_centre->y / c.resolution.y));
        }
        glUniform4fv(glGetUniformLocation(shaders[S_POST], "light"), (int)(light_sources.size()), reinterpret_cast<GLfloat *>(light_sources.data()));
        glUniform1i(glGetUniformLocation(shaders[S_POST], "light_size"), (int)(light_sources.size()));
        glUniform1f(glGetUniformLocation(shaders[S_POST], "light_distortion"), (float)c.resolution.x / (float)c.resolution.y);
        glUniform1i(glGetUniformLocation(shaders[S_POST], "use_light"), true);
    } else {
        glUniform1i(glGetUniformLocation(shaders[S_POST], "use_light"), false);
    }
#endif
    
    //Set texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_render);
    glUniform1i(glGetUniformLocation(shaders[S_POST], "tex"), 1);
    
    //Vertices
    glBindVertexArray(vao[V_POST]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_POST]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //Matrices
    glUniformMatrix4fv(mvp[S_POST], 1, GL_FALSE, glm::value_ptr(proj_post));
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Post: %d", e);
    }
}
//-----------------------------------------



//PHASE 3: SMOOTH CAMERA AND RESIZE
//fb_cam, tex_cam (c.resolution * c.render_scale) using shaders[S_CAM]
//-----------------------------------------
void Graphics::Renderer::renderCam(Config &c) {
    //Render Target: fb_cam
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glUseProgram(shaders[S_CAM]);
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_post);
    glUniform1i(glGetUniformLocation(shaders[S_CAM], "tex"), 0);
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //Matrices
    mat4 model = matModel2D(Vec2(-c.render_scale, -c.render_scale), (c.resolution + Vec2(2,2)) * c.render_scale);
    glUniformMatrix4fv(mvp[S_CAM], 1, GL_FALSE, glm::value_ptr(proj_cam * *m_e_camera * model));
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Cam and Resize: %d", e);
    }
}
//-----------------------------------------



//PHASE 4: RENDER TO WINDOW
//directly to window (c.window_size) using shaders[S_WINDOW]
//-----------------------------------------
void Graphics::Renderer::renderWindow(Config &c) {
    //Render Target: Window (fb 0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shaders[S_WINDOW]);
    
    //Palette for background
    Graphics::Palette::render(c, palette_tex, shaders[S_WINDOW]);
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_cam);
    glUniform1i(glGetUniformLocation(shaders[S_WINDOW], "tex"), 0);
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //Matrices
    mat4 model = matModel2D((c.window_size - c.resolution * c.render_scale) * 0.5f, c.resolution * c.render_scale);
    mat4 modelbg = matModel2D(Vec2(0,0), c.window_size);
    
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
    
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR) {
       log::error("OpenGL Error during Window: %d", e);
    }
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
void Graphics::Renderer::clear(Scene &scene, Config &c) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(c.background_color[0], c.background_color[1], c.background_color[2], c.background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
        
    if (previous_window_size.x != c.window_size.x or previous_window_size.y != c.window_size.y) {
        previous_window_size = c.window_size;
        
        glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
        
        glBindTexture(GL_TEXTURE_2D, tex_cam);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c.resolution.x * c.render_scale, c.resolution.y * c.render_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_cam, 0);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            log::error("Error creating Game Framebuffer: %d", glGetError());
            SDL_Quit();
            exit(-1);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        proj_cam = ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
        proj_window = ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
        
#ifdef TILEMAP
        System::Tilemap::init(scene, c);
#endif
    }
}
//-----------------------------------------



//DESTROY
//-----------------------------------------
void Graphics::Renderer::destroy() {
    //OPENGL
    
    //IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    //SDL
    SDL_GL_DeleteContext(context);
}
//-----------------------------------------



//HELPERS
//-----------------------------------------
void Graphics::Renderer::bindCamera(glm::mat4 *mat, glm::mat4 *mat_e, Vec2f *pos) {
    m_camera = mat;
    m_e_camera = mat_e;
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

ui32 Graphics::Renderer::createTexture(ui8* tex, int w, int h, bool rgba) {
    ui32 tex_id;
    
    glGenTextures(1, &tex_id);
    
    glBindTexture(GL_TEXTURE_2D, tex_id);
    if (rgba) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, tex);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  
    return tex_id;
}

void Graphics::Renderer::prepareTilemap(Rect2 &dst, Config &c, std::array<float, 24> &vertices) {
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
