//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "scene.h"
#include <vector>

namespace Verse
{
    struct Config {
        str name;
        str version;
        
        Vec2 resolution;
        Vec2 window_size;
        int render_scale;
        bool enable_gui;
        bool use_vsync;
        
        float game_speed;
        ui16 fps;
        ui32 physics_time;
        ui32 render_time;
        float physics_delta;
        
        bool use_grayscale;
        bool use_light;
        int palette_index;
        int num_palettes;
        float background_color[4];
        
        bool render_collision_boxes;
        
        Scene* active_scene;
        Component::Camera* active_camera;
        
        float gravity;
        Vec2f gravity_dir;
    };
}
