//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//fresa, version 0.2

#pragma once

#include "dtypes.h"
#include "scene.h"
#include <vector>

namespace Verse
{
    struct Config {
        str name;
        ui8 version[3];
        
        Vec2 resolution;
        
        SDL_Window* window;
        Vec2 window_size;
        ui16 render_scale;
        ui16 refresh_rate;
        bool use_vsync;
        
        ui16 fps;
        float physics_time;
        float render_time;
        double timestep;
        double physics_delta;
        double physics_interpolation;
        float game_speed;
        
        bool use_grayscale;
        bool use_light;
        ui16 palette_index;
        ui16 num_palettes;
        float background_color[4];
        
        bool render_collision_boxes;
        
        bool tme_active;
        Component::Tilemap* tme_curr_tmap;
        EntityID tme_curr_id;
        ui8 tme_curr_tile;
        
        Scene* active_scene;
        Component::Camera* active_camera;
        
        bool enable_lookahead;
        bool enable_smooth_panning;
        bool use_subpixel_cam;
        
        float gravity;
        Vec2f gravity_dir;
        
        bool player_loses_light;
    };
}
