//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>

#include "dtypes.h"
#include "log.h"
#include "scene.h"
#include "config.h"
#include "r_pipeline.h"

#define TIMESTEP 10 //Update the simulation every 10ms (0.01s)
#define DELTA (TIMESTEP * 0.001f)

namespace Verse::Game
{
    bool init(Config &c);

    void setActiveScene(Scene* active_scene);

    bool update();
    bool physicsUpdate();
    void timeFrame();

    void stop();
}
