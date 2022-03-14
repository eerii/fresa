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
    
    template <typename UBO, typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    DrawDescription getDrawDescription(const std::vector<V> &vertices, const std::vector<I> &indices, ShaderID shader, TextureID texture = no_texture) {
        DrawDescription description{};
        
        description.shader = shader;
        description.texture = texture;
        description.uniform = API::registerDrawUniforms<UBO>(api, shader);
        description.vertex.emplace<0>(API::registerGeometryBuffer(api, vertices, indices));
        
        API::updateDescriptorSets(api, description);
        return description;
    }

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);

    void draw(DrawDescription &description, glm::mat4 model);
    
    void updateCameraView(CameraData &cam);
    void updateCameraProjection(CameraData &cam);
}
