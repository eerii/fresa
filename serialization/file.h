//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#include "types.h"
#include <optional>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Fresa::File
{
    void init();
    str path(str p);
    std::optional<str> path_optional(str p);
}
