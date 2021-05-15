//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "file.h"

#include <fstream>
#include <streambuf>

using namespace Verse;

void File::init() {
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        log::error("Couldn't get file system representation!");
    }
    CFRelease(resourcesURL);
    chdir(path);
    log::debug("Path: %s", path);
#endif
}
