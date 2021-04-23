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
#ifdef TEXTURE
    void loadTexture(str path, Component::Texture* tex);
#endif
#ifdef TILEMAP
    void loadTexture(str path, Component::Tilemap* tex);
#endif
}
