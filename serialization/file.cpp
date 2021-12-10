//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "file.h"

#include <fstream>
#include <streambuf>

#include "log.h"

using namespace Fresa;

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
    //TODO: Add support for Linux and Windows
}
