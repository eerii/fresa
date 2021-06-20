//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_opengl.h"
#include "c_text.h"

namespace Verse::Graphics::Font
{
    void load(FontInfo* font, str path);
    void render(Component::Text* text, Vec2 size, int line_height);
}
