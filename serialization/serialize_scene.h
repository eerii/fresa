//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "log.h"
#include "serialization.h"
#include "scene.h"
#include "config.h"

namespace Verse::Serialization
{
    void loadScene(str name, Scene &s, Config &c);
}
