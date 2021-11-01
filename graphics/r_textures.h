//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"
#include "component_list.h"

#include "r_renderdata.h"
#include "r_texturedata.h"

#include "stb_image.h"
#include "stb_image_write.h"

namespace Verse::Graphics::Texture
{
    enum Channels {
        TEXTURE_CHANNELS_G = 1,
        TEXTURE_CHANNELS_GA = 2,
        TEXTURE_CHANNELS_RGB = 3,
        TEXTURE_CHANNELS_RGBA = 4
    };

    TextureData load(GraphicsAPI &api, str path, Channels ch = TEXTURE_CHANNELS_RGBA);

    //TODO: DEPRECATE
    //----------------------------------------
    void loadTexture(str path, Component::Texture* tex);
    void loadTexture(std::vector<str> path, Component::Tilemap* tex);

    void createTexture(ui8* tex, TextureData &data, bool rgba = true);
    void createTexture(ui8* tex, TextureData &data, int w, int h, bool rgba = true);

    void createPerlinNoise(ui8* noise_data, TextureData &tex_data, Vec2<> size, Vec2<> offset, float freq, int levels);
    //----------------------------------------
}
