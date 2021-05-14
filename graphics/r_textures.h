//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "stb_image.h"

#include "dtypes.h"
#include "log.h"
#include "config.h"

#include "component_list.h"

namespace Verse::Graphics::Texture
{
    void loadTexture(str path, ui32 &tex_id);

    //void createWhiteNoise(int size, ui8* noise_data, ui32 &tex_id);
    //void offsetWhiteNoise(int size, ui8* noise_data, ui32 &tex_id);
    void createPerlinNoise(Vec2 size, Vec2 offset, float freq, int octaves, ui32 seed, ui8* noise_data, ui32 &tex_id);
    void createGradient(int size, ui32 &tex_id);

#ifdef TEXTURE
    void loadTexture(str path, Component::Texture* tex);
#endif
#ifdef TILEMAP
    void loadTexture(str path, Component::Tilemap* tex);
#endif
}
