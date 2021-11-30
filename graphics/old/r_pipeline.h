//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Fresa::Graphics
{
    bool init(Config &c);

    void render(Config &c);
    void clear(Config &c);
    void display(Config &c);

    void destroy();
}
