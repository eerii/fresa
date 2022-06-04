//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "types.h"

namespace Fresa::Audio
{
    struct AudioAPI {
        SDL_AudioSpec spec;
        SDL_AudioDeviceID device;
    };
    inline AudioAPI audio_api;
    
    struct Sound {
        ui32 length;
        ui32 remainder;
        ui8* loc;
        ui8* buffer;
        ui8 volume;
        bool loop;
    };
    using SoundID = ui16;
    inline std::map<SoundID, Sound> sounds{};
    
    inline std::vector<SoundID> playlist{};
    
    //---
    
    void init();
    
    void callback(void* userdata, ui8* stream, int len);
    
    void pause();
    void unpause();
    
    SoundID load(str file, ui8 volume, bool loop);
    void unload(SoundID sound);
    
    void play(SoundID sound);
    void stop(SoundID sound);

    void destroy();
}
