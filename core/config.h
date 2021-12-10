//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "scene.h"
#include "component_list.h"

namespace Fresa::Conf
{
    inline const str name = "Proxecto Fresa";
    inline const ui8 version[3] = {0, 3, 1};
    inline const Vec2<> window_size = Vec2(1024, 720);
    inline const Vec2<> resolution = Vec2(256, 180);
    
    inline const float timestep = 10.0f;
    inline float game_speed = 1.0f;
}

namespace Fresa
{
    struct Config {
        Vec2<> resolution;
        
        SDL_Window* window;
        Vec2<> window_size;
        ui16 render_scale;
        ui16 refresh_rate;
        
        Scene* active_scene;
        //Component::Camera* active_camera;
        
        bool enable_lookahead;
        bool enable_smooth_panning;
        bool use_subpixel_cam;
        
        float gravity;
        Vec2<float> gravity_dir;
        
        bool player_loses_light;
    };
}
