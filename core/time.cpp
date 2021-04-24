//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "time.h"

using namespace Verse;

ui64 Time::current = 0;
ui64 Time::previous = 0;
ui64 Time::delta = 0;

ui64 Verse::time() {
    return (ui64)SDL_GetTicks();
}
