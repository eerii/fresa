//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_palette.h"
#include "r_opengl.h"
#include "time.h"

#define TRANSITION_TIME 500

using namespace Verse;

namespace {
    ui16 previous_palette = 0;
    ui64 switch_palette_time = 0;
    float transition_percent = 0.0;
}

void Graphics::Palette::render(Config &c, ui32 &palette_tex, ui8 &pid) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, palette_tex);
    glUniform1i(glGetUniformLocation(pid, "palette"), 1);
    
    if (previous_palette != c.palette_index) {
        if (switch_palette_time == 0) {
            switch_palette_time = Time::current;
        } else {
            int delay = int(Time::current - switch_palette_time);
            
            if (delay < TRANSITION_TIME) {
                transition_percent = (float)delay / (float)TRANSITION_TIME;
            } else {
                switch_palette_time = 0; transition_percent = 0.0;
                previous_palette = c.palette_index;
            }
        }
    } else if (switch_palette_time != 0 or transition_percent != 0.0) {
        switch_palette_time = 0; transition_percent = 0.0;
    }
    
    if (c.palette_index > -1.0f) {
        glUniform1f(glGetUniformLocation(pid, "palette_index"), ((float)c.palette_index / (float)c.num_palettes));
        glUniform1f(glGetUniformLocation(pid, "previous_palette_index"), ((float)previous_palette / (float)c.num_palettes) + 0.001);
        glUniform1f(glGetUniformLocation(pid, "transition_percent"), transition_percent);
        glUniform1i(glGetUniformLocation(pid, "use_grayscale"), c.use_grayscale);
    } else {
        glUniform1f(glGetUniformLocation(pid, "palette_index"), -1.0f);
    }
}
