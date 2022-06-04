//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "types.h"

namespace Fresa {
    struct Config {
        static const str name;
        static const ui8 version[3];
        static const Vec2<ui16> window_size;
        static const Vec2<ui16> resolution;
        
        static const float timestep;
        static float game_speed;
        
        static str renderer_description_path;
        static ui8 multisampling;
        
        static constexpr ui8 frames_in_flight = 2;
        static bool prefer_mailbox_mode;
    };
}
