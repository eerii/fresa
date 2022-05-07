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
    
    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);
}
