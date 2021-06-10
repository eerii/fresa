//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <vector>

#include "config.h"
#include "component_list.h"

#include "stb_image.h"

namespace Verse::Graphics::Texture
{
    void loadTexture(str path, ui32 &tex_id);
    void loadTexture(str path, Component::Texture* tex);
    void loadTexture(std::vector<str> path, Component::Tilemap* tex);

    void createPerlinNoise(Vec2 size, Vec2 offset, float freq, int octaves, ui32 seed, ui8* noise_data, ui32 &tex_id);
    void createGradient(int size, ui32 &tex_id);
}
