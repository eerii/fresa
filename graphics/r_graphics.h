//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

//---Graphics---
//      This is fresa's API for graphics, what is meant to be used when designing games
//      It is intended to be as straightforward as possible, allowing for texture and vertex data registration for later use
//      Read more about each specific function in the .cpp file

namespace Fresa::Graphics
{
    inline WindowData win;
    inline CameraData camera;
    inline GraphicsAPI api;
    
    bool init();
    bool update();
    bool stop();
    
    void onResize(Vec2<> size);
    inline Event::Event<Vec2<>> event_window_resize;
    inline Event::Observer observer = event_window_resize.createObserver(onResize);
    
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    DrawID getDrawID(const std::vector<V> &vertices, const std::vector<ui16> &indices, Shaders shader = SHADER_DRAW_TEX) {
        DrawBufferID buffer = API::registerDrawBuffer(api, vertices, indices);
        return API::registerDrawData(api, buffer, shader);
    }
    
    DrawID getDrawID_Rect(Shaders shader = SHADER_DRAW_TEX);
    DrawID getDrawID_Cube(Shaders shader = SHADER_DRAW_COLOR);

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);
    void bindTexture(DrawID draw_id, TextureID texture_id);
    void unbindTexture(DrawID draw_id);

    void draw(const DrawID draw_id, glm::mat4 model);
    
    void updateCameraView(CameraData &cam);
    void updateCameraProjection(CameraData &cam);
    
    //---Vertices---
    //      Some common vertex definitions that can be used for quickly creating objects
    namespace Vertices {
        inline const std::vector<VertexDataTexture> rect_vertices_texture = {
            {{0.f, 0.f, 0.f}, {0.0f, 0.0f}},
            {{1.f, 0.f, 0.f}, {1.0f, 0.0f}},
            {{1.f, 1.f, 0.f}, {1.0f, 1.0f}},
            {{0.f, 1.f, 0.f}, {0.0f, 1.0f}},
        };
        
        inline const std::vector<VertexDataColor> rect_vertices_color = {
            {{0.f, 0.f, 0.f}, {1.0f, 0.0f, 0.0f}},
            {{1.f, 0.f, 0.f}, {0.0f, 1.0f, 0.0f}},
            {{1.f, 1.f, 0.f}, {0.0f, 0.0f, 1.0f}},
            {{0.f, 1.f, 0.f}, {1.0f, 1.0f, 1.0f}},
        };
        
        inline const std::vector<ui16> rect_indices = {
            0, 2, 1, 0, 3, 2
        };
        
        //: Example colored cube
        inline const std::vector<VertexDataColor> cube_vertices_color = {
            {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}}, //Light
            {{1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}}, //Teal
            {{1.f, 1.f, -1.f}, {1.000f, 0.815f, 0.019f}}, //Yellow
            {{-1.f, 1.f, -1.f}, {0.988f, 0.521f, 0.113f}}, //Orange
            {{-1.f, -1.f, 1.f}, {0.925f, 0.254f, 0.345f}}, //Red
            {{1.f, -1.f, 1.f}, {0.925f, 0.235f, 0.647f}}, //Pink
            {{1.f, 1.f, 1.f}, {0.658f, 0.180f, 0.898f}}, //Purple
            {{-1.f, 1.f, 1.f}, {0.258f, 0.376f, 0.941f}}, //Blue
        };

        inline const std::vector<ui16> cube_indices = {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1,
            5, 4, 6, 6, 4, 7,
        };
    }
}
