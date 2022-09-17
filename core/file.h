//* file
//      contains utilities related to file handling
#pragma once

#include "fresa_config.h"
#include "log.h"
#include "engine.h"
#include "system.h"

#include <optional>
#include <filesystem>
#include <fstream>

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

namespace fs = std::filesystem;

namespace fresa::file
{
    //* file system
    struct FileSystem {
        inline static System<FileSystem, system::SystemPriority::FILES> system;

        //: initialize files
        //      necessary for macos, creates a corefoundation bundle
        static void init() {
            #ifdef __APPLE__
            CFBundleRef mainBundle = CFBundleGetMainBundle();
            CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
            char path[PATH_MAX];
            if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
                log::error("couldn't get file system representation!");
            CFRelease(resourcesURL);
            chdir(path);
            #endif
        }
    };

    //* returns the absolute path to a file, relative to the executable, while also checking if the file exists
    inline str path(str_view p) {
        //: join the resource path with the given path
        str full_path = fmt::format("{}/{}", engine_config.res_path(), p);

        //: check if the file exists
        if (not fs::exists(full_path)) {
            log::error("file '{}' does not exist", full_path);
            fresa::quit();
            return "";
        }

        //: return the complete path
        return full_path;
    }

    //* returns the path to a file, does not fail, instead returns an empty optional
    inline std::optional<str> optional_path(str_view p) {
        //: join the resource path with the given path
        std::optional<str> full_path = fmt::format("{}/{}", engine_config.res_path(), p);

        //: return the path or an empty optional
        return fs::exists(full_path.value()) ? full_path : std::nullopt;
    }
}