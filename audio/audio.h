//project verse, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "types.h"

namespace Fresa::Audio
{
    struct AudioAPI {
        SDL_AudioSpec spec;
        SDL_AudioDeviceID device;
    };
    inline AudioAPI api;
    
    struct Sound {
        ui32 length;
        ui8* buffer;
        ui8 volume;
    };
    using SoundID = ui8;
    inline std::map<SoundID, Sound> sounds{};
    
    void init();
    
    void callback(void* userdata, ui8* stream, int len);
    
    void pause();
    void unpause();
    
    SoundID load(str file, ui8 volume);
}
