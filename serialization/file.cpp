//: fresa by jose pazos perez, licensed under GPLv3
#include "file.h"

#include <streambuf>

#include "log.h"

using namespace Fresa;

namespace {
    #ifdef __EMSCRIPTEN__
    str base_path = "";
    #else
    str base_path = "res/";
    #endif
}

void File::init() {
#ifdef __APPLE__
    //---Init filesystem in MacOS---
    //      CoreFoundation needs to be initialized and add a resource url to be able to load the "res" folder
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        log::error("Couldn't get file system representation!");
    CFRelease(resourcesURL);
    chdir(path);
    log::debug("Path: %s", path);
#endif
}

str File::path(str p) {
    str full_path = base_path + p;
    
    if (not fs::exists(full_path))
        log::error("Tried to access a path that does not exist: %s", full_path.c_str());
    
    return full_path;
}

std::optional<str> File::path_optional(str p) {
    str full_path = base_path + p;
    
    if (fs::exists(full_path))
        return full_path;
    return std::nullopt;
}
