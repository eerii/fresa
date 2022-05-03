//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

#include "r_window.h"
#include "r_camera.h"

//---Graphics---
//      This is fresa's API for graphics, what is meant to be used when designing games
//      It is intended to be as straightforward as possible, allowing for texture and vertex data registration for later use
//      Read more about each specific function in the .cpp file

namespace Fresa::Graphics
{
    bool init();
    bool update();
    bool stop();
    
    template <typename... UBO, typename V, typename U, typename I,
              std::enable_if_t<Reflection::is_reflectable<V> && Reflection::is_reflectable<U> && std::is_integral_v<I>, bool> = true>
    DrawDescription getDrawDescription(const std::vector<V> &vertices, const std::vector<U> &instanced_data, const std::vector<I> &indices,
                                       ShaderID shader) {
        DrawDescription description{};
        description.mesh = Buffer::registerMesh(vertices, indices);
        
        Common::updateBuffer(storage_buffers.at(key_storage_buffers.at("TransformBuffer")),
                             (void*)instanced_data.data(), (ui32)(instanced_data.size() * sizeof(U)));
        
        return description;
    }
    
    void draw(DrawDescription &description, ShaderID shader);

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);

    void draw(DrawDescription &description, glm::mat4 model);
}
