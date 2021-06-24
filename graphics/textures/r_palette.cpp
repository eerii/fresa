//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_palette.h"

#include "ftime.h"
#include "r_opengl.h"

#define TRANSITION_TIME 500

using namespace Verse;

namespace {
    ui16 previous_palette = 0;
    ui32 switch_palette_time = 0;
    float transition_percent = 0.0f;
    float palette_interval = 1.0f;
}

void Graphics::Palette::render(Config &c, ui32 &palette_tex, ui8 &pid) {
    glUseProgram(pid);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, palette_tex);
    glUniform1i(glGetUniformLocation(pid, "palette"), 2);
    
    switchPalette(c);
    
    float p_i = (c.palette_index > -1.0f) ? (float)c.palette_index / (float)c.num_palettes : -1.0f;
    glUniform1f(glGetUniformLocation(pid, "palette_index"), p_i);
    
    if (c.palette_index > -1.0f) {
        glUniform1f(glGetUniformLocation(pid, "previous_palette_index"), ((float)previous_palette / (float)c.num_palettes) + 0.001);
        glUniform1f(glGetUniformLocation(pid, "palette_interval"), palette_interval);
        glUniform1f(glGetUniformLocation(pid, "transition_percent"), transition_percent);
        glUniform1i(glGetUniformLocation(pid, "use_grayscale"), c.use_grayscale);
    }
}

void Verse::Graphics::Palette::switchPalette(Config &c) {
    if (previous_palette == c.palette_index) {
        switch_palette_time = 0;
        transition_percent = 0.0;
        return;
    }
    
    if (switch_palette_time == 0) {
        switch_palette_time = time();
        return;
    }
    
    ui32 delay = time() - switch_palette_time;
    
    if (delay < TRANSITION_TIME) {
        transition_percent = (float)delay / (float)TRANSITION_TIME;
        return;
    }
    
    switch_palette_time = 0; transition_percent = 0.0;
    previous_palette = c.palette_index;
}

void Verse::Graphics::Palette::setPaletteInterval(int w) {
    if (w == 0)
        return;
    palette_interval = 1.0f / (float)w;
}
