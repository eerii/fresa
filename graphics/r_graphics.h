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
    
    template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    DrawID getDrawID(const std::vector<V> &vertices, const std::vector<I> &indices, ShaderID shader) {
        DrawBufferID buffer = API::registerDrawBuffer(api, vertices, indices);
        DrawID id = API::registerDrawData(api, buffer, shader);
        API::updateDescriptorSets(api, &API::draw_data.at(id));
        return id;
    }

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);
    void bindTexture(DrawID draw_id, TextureID texture_id);
    void unbindTexture(DrawID draw_id);

    void draw(const DrawID draw_id, glm::mat4 model);
    
    void updateCameraView(CameraData &cam);
    void updateCameraProjection(CameraData &cam);
}
