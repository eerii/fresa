//: fresa by jose pazos perez, licensed under GPLv3
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
    requested_spec.userdata = nullptr;
    
    audio_api.device = SDL_OpenAudioDevice(nullptr, 0, &requested_spec, &audio_api.spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (audio_api.device == 0)
        log::error("Failed to open audio device, %s", SDL_GetError());
    
    if (audio_api.spec.format != requested_spec.format)
        log::warn("The audio format was not the requested one");
    
    unpause();
}

void Audio::callback(void *userdata, ui8 *stream, int len) {
    SDL_memset(stream, 0, len);
    
    for (auto id : playlist) {
        Sound &sound = sounds.at(id);
        
        if (sound.remainder > 0) {
            ui32 length = (ui32)len > sound.remainder ? sound.remainder : (ui32) len;
            
            SDL_MixAudioFormat(stream, sound.buffer, audio_api.spec.format, length, sound.volume);
            sound.buffer += length;
            sound.remainder -= length;
        } else if (sound.loop) {
            sound.buffer = sound.loc;
            sound.remainder = sound.length;
        } else {
            stop(id);
        }
    }
}

void Audio::pause() {
    SDL_PauseAudioDevice(audio_api.device, 1);
}

void Audio::unpause() {
    SDL_PauseAudioDevice(audio_api.device, 0);
}

Audio::SoundID Audio::load(str file, ui8 volume, bool loop) {
    static SoundID id = 0;
    while (Audio::sounds.find(id) != Audio::sounds.end())
        id++;
    
    //: Create sound
    Audio::sounds[id] = Sound{};
    Sound& s = Audio::sounds.at(id);
    
    s.volume = volume;
    s.loop = loop;
    
    //: Load file
    file = File::path("audio/" + file);
    str extension = split(file, ".").back();
    
    if (extension == "wav") {
        if (SDL_LoadWAV(file.c_str(), &audio_api.spec, &s.loc, &s.length) == NULL)
            log::error("Error loading the audio file %s", file.c_str());
        s.buffer = s.loc;
        s.remainder = s.length;
    } else if (extension == "ogg") {
        log::error("Audio extension .ogg is not implemented yet");
    } else {
        log::error("Unsupported audio extension .%s", extension.c_str());
    }
    
    return id;
}

void Audio::unload(SoundID sound) {
    SDL_FreeWAV(Audio::sounds.at(sound).loc);
    Audio::sounds.erase(sound);
}

void Audio::play(SoundID sound) {
    if (not std::count(playlist.begin(), playlist.end(), sound)) //: Add sound if it is not already playing
        playlist.push_back(sound);
}

void Audio::stop(SoundID sound) {
    auto it = std::find(playlist.begin(), playlist.end(), sound);
    if (it != playlist.end())
        playlist.erase(it);
}

void Audio::destroy() {
    Audio::pause();
    SDL_CloseAudioDevice(audio_api.device);
}
