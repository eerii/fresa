//project verse, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "audio.h"
#include "log.h"
#include "file.h"

using namespace Fresa;

void Audio::init() {
    //: Audio description
    SDL_AudioSpec requested_spec;
    requested_spec.freq = 48000;
    requested_spec.format = AUDIO_S16;
    requested_spec.channels = 2;
    requested_spec.samples = 4096;
    requested_spec.callback = callback;
    requested_spec.userdata = malloc(sizeof(Sound));
    
    api.device = SDL_OpenAudioDevice(nullptr, 0, &requested_spec, &api.spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (api.device == 0)
        log::error("Failed to open audio device, %s", SDL_GetError());
    
    if (api.spec.format != requested_spec.format)
        log::warn("The audio format was not the requested one");
    
    unpause();
}

void Audio::callback(void *userdata, ui8 *stream, int len) {
    Sound* sound = (Sound*) userdata;
    
    SDL_memset(stream, 0, len);
    
    if (sound != nullptr) {
        if (sound->length > 0) {
            ui32 length = (ui32)len > sound->length ? sound->length : (ui32) len;
            
            SDL_MixAudioFormat(stream, sound->buffer, api.spec.format, length, sound->volume);
            sound->buffer += length;
            sound->length -= length;
        }
    }
}

void Audio::pause() {
    SDL_PauseAudioDevice(api.device, 1);
}

void Audio::unpause() {
    SDL_PauseAudioDevice(api.device, 0);
}

Audio::SoundID Audio::load(str file, ui8 volume) {
    static SoundID id = 0;
    while (Audio::sounds.find(id) != Audio::sounds.end())
        id++;
    
    //: Create sound
    Audio::sounds[id] = Sound{};
    Sound& s = Audio::sounds.at(id);
    
    s.volume = volume;
    
    //: Load file
    File::path(file);
    str extension = split(file, ".").back();
    
    if (extension == "wav") {
        if (SDL_LoadWAV(file.c_str(), &api.spec, &s.buffer, &s.length) == NULL)
            log::error("Error loading the audio file %s", file.c_str());
        
        //...
    } else if (extension == "ogg") {
        log::error("Audio extension .ogg is not implemented yet");
    } else {
        log::error("Unsupported audio extension .%s", extension.c_str());
    }
    
    return id;
}
