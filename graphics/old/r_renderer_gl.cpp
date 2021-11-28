//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_renderer.h"
#include "r_opengl_api.h"

#include <glm/ext.hpp>

#include "log.h"

#include "gui.h"

#include "r_shader.h"
#include "r_palette.h"
#include "r_window.h"
#include "system_list.h"

#include "r_api.h"

#define CLIPPING_FAR -1000.0f
#define CLIPPING_NEAR 1000.0f

using namespace Verse;
using namespace Graphics;
using namespace glm;

namespace {
    OpenGL *gl;

    //FRAMEBUFFERS
    ui32 fb_render, fb_light, fb_post, fb_cam;
    ui32 tex_render, tex_light, tex_post, tex_cam;
    ui32 depth_tex_render;

    //VERTICES
    enum VBOs {
        V_EMPTY = 0,
        V_TEX,
        V_TILE,
        V_NOISE,
        V_TEXT,
        V_LIGHT,
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

    //MATRICES
    mat4 proj_render, proj_post, proj_cam, proj_window;

    //MISC
    ui32 palette_tex;
    Vec2 previous_window_size;
}



//CREATE
//-----------------------------------------
void Graphics::Renderer::create(OpenGL *api, Config &c) {
    log::debug("Creating renderer");
    
    gl = api;
    
    //FRAMEBUFFERS
    //-----------------------------------------
    //Phase 1: Render to small texture
    Renderer::createDepthFramebuffer(c, fb_render, tex_render, depth_tex_render, c.resolution + Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH));
    
    //Phase 2: Postprocessing effects
    Renderer::createFramebuffer(c, fb_light, tex_light, c.resolution + Vec2(2*BORDER_WIDTH ,2*BORDER_WIDTH));
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
    for (auto &[key, s] : gl->shaders)
        Shader::validate(s);
    
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
    
    //Phase 1c: Render noise
    glBindVertexArray(vao[V_NOISE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_NOISE]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    //Phase 1d: Render text
    glBindVertexArray(vao[V_TEXT]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEXT]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //Phase 2: Post
    glBindVertexArray(vao[V_LIGHT]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_LIGHT]);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
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
    proj_render = ortho(0.0f, (float)c.resolution.x + 2*BORDER_WIDTH, (float)c.resolution.y + 2*BORDER_WIDTH, 0.0f, CLIPPING_NEAR, CLIPPING_FAR);
    proj_post = ortho(0.0f, 1.0f, 0.0f, 1.0f);
    proj_cam = ortho(0.0f, (float)(c.resolution.x * c.render_scale), 0.0f, (float)(c.resolution.y * c.render_scale));
    proj_window = ortho(0.0f, (float)(c.window_size.x), 0.0f, (float)(c.window_size.y));
    //-----------------------------------------
    glCheckError();
    
    //BLEND ALPHA
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCheckError();
    
    //PALETTE TEX
    int w, h, ch;
    ui8* tex = stbi_load("res/graphics/palette.png", &w, &h, &ch, STBI_rgb_alpha);
    TextureData palette_data;
    Graphics::Texture::createTexture(tex, palette_data, w, h);
    palette_tex = palette_data.id_; //TODO: CHANGE THIS
    Palette::setPaletteInterval(w);
    glCheckError();
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glCheckError();
}
//-----------------------------------------



//PHASE 1: RENDER TO SMALL TEXTURE
//fb_render, tex_render (c.resolution) using shaders[S_RENDER2D] or shaders[S_RENDER3D]
//-----------------------------------------
void Graphics::Renderer::renderTexture(Config &c, TextureData &data) {
    /*//Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(gl->shaders["render"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_TEX]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.vertices.size(), data.vertices.data(), GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(gl->shaders["render"].locations["layer"], (float)data.layer);
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering texture)");
    glUniformMatrix4fv(gl->shaders["render"].locations["mvp"], 1, GL_FALSE,
                       glm::value_ptr(proj_render * c.active_camera->m_pixel * data.model));
    glCheckError();
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, data.id_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}

void Graphics::Renderer::renderTilemap(Config &c, TextureData &data) {
    /*//Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(gl->shaders["render"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_TILE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TILE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.vertices.size(), data.vertices.data(), GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(gl->shaders["render"].locations["layer"], (float)data.layer);
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering tilemap)");
    glUniformMatrix4fv(gl->shaders["render"].locations["mvp"], 1, GL_FALSE,
                       glm::value_ptr(proj_render * c.active_camera->m_pixel * data.model));
    glCheckError();
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, data.id_);
    glDrawArrays(GL_TRIANGLES, 0, (int)(data.vertices.size() / 4));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}

void Graphics::Renderer::renderNoise(Config &c, TextureData &mask_data, TextureData &noise_data) {
    /*//Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(gl->shaders["noise"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_NOISE]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mask_data.vertices.size(), mask_data.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_NOISE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * noise_data.vertices.size(), noise_data.vertices.data(), GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(gl->shaders["noise"].locations["layer"], (float)mask_data.layer);
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering texture)");
    glUniformMatrix4fv(gl->shaders["noise"].locations["mvp"], 1, GL_FALSE,
                       glm::value_ptr(proj_render * c.active_camera->m_pixel * mask_data.model));
    glCheckError();
    
    //Textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noise_data.id_);
    glUniform1i(gl->shaders["noise"].locations["noise"], 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mask_data.id_);
    glUniform1i(gl->shaders["noise"].locations["mask"], 2);
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}

void Graphics::Renderer::renderText(Config &c, TextureData &data, float r, float g, float b, bool same_color) {
    /*//Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_render);
    glUseProgram(gl->shaders["text"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_TEXT]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_TEXT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.vertices.size(), data.vertices.data(), GL_STATIC_DRAW);
    glCheckError();
    
    //Layer
    glUniform1f(gl->shaders["text"].locations["layer"], (float)data.layer);
    
    //Color
    glUniform3f(gl->shaders["text"].locations["text_color"], r, g, b);
    glUniform1i(gl->shaders["text"].locations["same_color"], same_color);
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering texture)");
    glUniformMatrix4fv(gl->shaders["text"].locations["mvp"], 1, GL_FALSE,
                       glm::value_ptr(proj_render * c.active_camera->m_pixel * data.model));
    glCheckError();
    
    //Draw
    glBindTexture(GL_TEXTURE_2D, data.id_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}
//-----------------------------------------



//PHASE 2: POSTPROCESSING
//fb_post, tex_post (c.resolution) using shaders[S_POST]
//-----------------------------------------
void Graphics::Renderer::renderLight(Config &c) {
    /*//Render Target: fb_render
    glBindFramebuffer(GL_FRAMEBUFFER, fb_light);
    glUseProgram(gl->shaders["light"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_LIGHT]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_LIGHT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Light
    std::vector<glm::vec4> light_data = System::Light::render(c, gl->shaders["light"].pid);
    glUniform4fv(gl->shaders["light"].locations["light"], (int)(light_data.size()), reinterpret_cast<GLfloat *>(light_data.data()));
    glUniform1i(gl->shaders["light"].locations["light_size"], (int)(light_data.size()));
    glUniform1f(gl->shaders["light"].locations["light_distortion"], (float)(c.resolution.x + 2*BORDER_WIDTH) / (float)(c.resolution.y + 2*BORDER_WIDTH));
    
    //Matrices
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering texture)");
    glUniformMatrix4fv(gl->shaders["light"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_post));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}

void Graphics::Renderer::renderPost(Config &c) {
    /*//Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(gl->shaders["post"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_POST]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_POST]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Palette
    Graphics::Palette::render(c, palette_tex, gl->shaders["post"].pid);
    
    //Light
    glUniform1i(gl->shaders["post"].locations["use_light"], c.use_light);
    glUniform1i(gl->shaders["post"].locations["show_light"], c.show_light);
    
    //Set texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_render);
    glUniform1i(gl->shaders["post"].locations["tex"], 1);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, tex_light);
    glUniform1i(gl->shaders["post"].locations["light_tex"], 4);
    
    //Matrices
    glUniformMatrix4fv(gl->shaders["post"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_post));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}
//-----------------------------------------



//PHASE 3: SMOOTH CAMERA AND RESIZE
//fb_cam, tex_cam (c.resolution * c.render_scale) using shaders[S_CAM]
//-----------------------------------------
void Graphics::Renderer::renderCam(Config &c) {
    /*glViewport(0, 0, c.resolution.x * c.render_scale, c.resolution.y * c.render_scale);
    
    //Render Target: fb_cam
    glBindFramebuffer(GL_FRAMEBUFFER, fb_cam);
    glUseProgram(gl->shaders["cam"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_post);
    glUniform1i(gl->shaders["cam"].locations["tex"], 0);
    
    //Matrices
    mat4 model = matModel2D(Vec2(-c.render_scale*BORDER_WIDTH, -c.render_scale*BORDER_WIDTH), (c.resolution + Vec2(2*BORDER_WIDTH, 2*BORDER_WIDTH)) * c.render_scale);
    if (c.active_camera == nullptr)
        log::error("No active camera! (Rendering extra camera)");
    glUniformMatrix4fv(gl->shaders["cam"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_cam * c.active_camera->m_extra * model));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}
//-----------------------------------------



//PHASE 4: RENDER TO WINDOW
//directly to window (c.window_size) using shaders[S_WINDOW]
//-----------------------------------------
void Graphics::Renderer::renderWindow(Config &c) {
    /*glViewport(0, 0, c.window_size.x, c.window_size.y);
    
    //Render Target: Window (fb 0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(gl->shaders["window"].pid);
    glCheckError();
    
    //Vertices
    glBindVertexArray(vao[V_DRAW]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DRAW]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();
    
    //Palette for background
    Graphics::Palette::render(c, palette_tex, gl->shaders["window"].pid);
    
    //Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_cam);
    glUniform1i(gl->shaders["window"].locations["tex"], 0);
    
    //Matrices
    mat4 model = matModel2D((c.window_size - c.resolution * c.render_scale) * 0.5f, c.resolution * c.render_scale);
    mat4 modelbg = matModel2D(Vec2(0,0), c.window_size);
    glCheckError();
    
    //Draw Background
    glUniform1i(gl->shaders["window"].locations["is_background"], true);
    glUniformMatrix4fv(gl->shaders["window"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_window * modelbg));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //Draw Level
    glUniform1i(gl->shaders["window"].locations["is_background"], false);
    glUniformMatrix4fv(gl->shaders["window"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_window * model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}
//-----------------------------------------




//OTHER RENDERERS
//-----------------------------------------
void Graphics::Renderer::renderTest(WindowData &win) {
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(gl->shaders["cam"].pid);
    glCheckError();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, palette_tex);
    glUniform1i(gl->shaders["cam"].locations["tex"], 0);
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
    
    mat4 model = matModel2D(Vec2(0,0), win.size);
    glCheckError();
    
    glUniformMatrix4fv(gl->shaders["cam"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_window * model));
    glCheckError();
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glCheckError();*/
}

void Graphics::Renderer::renderDebugCollider(Config &c, Rect2<> col, bool colliding) {
    /*//Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(gl->shaders["debug"].pid);
    glCheckError();
    
    //Vertices
    float outlines[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    
    float epsilon_x = 1.0f / (float)col.w;
    outlines[0] += epsilon_x;
    float epsilon_y = 1.0f / (float)col.h;
    outlines[3] -= epsilon_y; outlines[5] -= epsilon_y;
    
    glBindVertexArray(vao[V_DEBUG]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DEBUG]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outlines), outlines, GL_STATIC_DRAW);
    glCheckError();
    
    //Color
    vec3 color = vec3(0.32f, 0.89f, 0.60f);
    if (colliding)
        color = vec3(0.98f, 0.27f, 0.42f);
    
    glUniform3f(gl->shaders["debug"].locations["c"], color.r, color.g, color.b);
    
    //Matrices
    mat4 model = matModel2D(col.pos() - Vec2(BORDER_WIDTH, BORDER_WIDTH), col.size());
    glUniformMatrix4fv(gl->shaders["debug"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
}

void Graphics::Renderer::renderDebugColliderCircle(Config &c, Vec2<> pos, ui16 radius, bool colliding) {
    /*//Render Target: fb_post
    glBindFramebuffer(GL_FRAMEBUFFER, fb_post);
    glUseProgram(gl->shaders["debug"].pid);
    glCheckError();
    
    //Vertices
    std::vector<vec2> vertices;
    int sides = (radius > 0) ? ((radius < 256) ? radius : 256) : 1;
    
    for (int a = 0; a < 360; a += 360 / sides) {
        float heading = a * 3.141592 / 180;
        vertices.push_back(vec2(cos(heading) * radius, sin(heading) * radius));
    }
    
    glBindVertexArray(vao[V_DEBUG]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[V_DEBUG]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glCheckError();
    
    //Color
    vec3 color = vec3(0.32f, 0.89f, 0.60f);
    if (colliding)
        color = vec3(0.98f, 0.27f, 0.42f);
    
    glUniform3f(gl->shaders["debug"].locations["c"], color.r, color.g, color.b);
    
    //Matrices
    mat4 model = matModel2D(pos - Vec2(BORDER_WIDTH, BORDER_WIDTH), Vec2(1,1));
    glUniformMatrix4fv(gl->shaders["debug"].locations["mvp"], 1, GL_FALSE, glm::value_ptr(proj_render * c.active_camera->m_pixel * model));
    glCheckError();
    
    //Draw
    glDrawArrays(GL_LINE_LOOP, 0, (int)vertices.size());
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glCheckError();*/
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
    glViewport(0, 0, c.resolution.x + 2*BORDER_WIDTH, c.resolution.y + 2*BORDER_WIDTH);
    
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
    
    System::Tilemap::init(c);
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
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
#endif
    
    //SDL
    SDL_GL_DeleteContext(gl->context);
}
//-----------------------------------------



//HELPERS
//-----------------------------------------
void Graphics::Renderer::onResize(WindowData &win) {
    
}

void Graphics::Renderer::createFramebuffer(Config &c, ui32 &fb, ui32 &tex, Vec2<> res) {
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

void Graphics::Renderer::createDepthFramebuffer(Config &c, ui32 &fb, ui32 &tex, ui32 &d_tex, Vec2<> res) {
    Graphics::Renderer::createFramebuffer(c, fb, tex, res);
    
    glGenTextures(1, &d_tex);
    glBindTexture(GL_TEXTURE_2D, d_tex);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, res.x, res.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d_tex, 0);
    glCheckError();
}

void Graphics::Renderer::toggleDepthTest(bool enable) {
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}
//-----------------------------------------



//MATRICES
//-----------------------------------------
mat4 Graphics::Renderer::matModel2D(Vec2<> pos, Vec2<> size, float rotation) {
    mat4 s = scale(mat4(1.0f), vec3(size.x, size.y, 1.0f));
    mat4 r = rotate(mat4(1.0f), rotation, vec3(0.0f, 0.0f, 1.0f));
    mat4 t = translate(mat4(1.0f), vec3(pos.x, pos.y, 0.0f));
    
    return t * r * s;
}
mat4 Graphics::Renderer::matModel2D(Rect2<> rect, float rotation) {
    return Graphics::Renderer::matModel2D(rect.pos(), rect.size(), rotation);
}

//-----------------------------------------

#endif
