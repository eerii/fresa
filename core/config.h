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
        bool use_subpixel_cam;
        
        ui16 fps;
        float physics_time;
        float render_time;
        double timestep;
        double physics_delta;
        double physics_interpolation;
        float game_speed;
        
        bool use_grayscale;
        bool use_light;
        int palette_index;
        int num_palettes;
        float background_color[4];
        
        bool render_collision_boxes;
        
        bool tme_active;
        Component::Tilemap* tme_curr_tmap;
        EntityID tme_curr_id;
        ui8 tme_curr_tile;
        
        Scene* active_scene;
        Component::Camera* active_camera;
        
        float gravity;
        Vec2f gravity_dir;
        
        bool player_loses_light;
    };
}
