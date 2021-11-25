//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_api.h"

namespace Verse::Graphics
{
    bool init();
    bool update();
    bool stop();

    void onResize(Vec2<> size);
}
