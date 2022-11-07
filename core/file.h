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

        //: io thread and queue
        inline static std::jthread io_thread;
        inline static std::queue<std::function<void()>> io_queue;

        //: initialize files
        //      necessary for macos, creates a corefoundation bundle
        //      create a thread for io operations
        static void init() {
            //: create io thread
            io_thread = std::jthread([](std::stop_token token) {
                while (!token.stop_requested()) {
                    while (!io_queue.empty()) {
                        if (token.stop_requested()) break;
                        auto f = io_queue.front();
                        io_queue.pop();
                        f();
                    }
                    std::this_thread::sleep_for(1s);
                }
            });

            //: macos specific code
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

        //: stop
        static void stop() {
            io_thread.request_stop();
            io_thread.detach();
        }

        //: add a job to the io queue
        static void add(std::function<void()> f) {
            io_queue.push(f);
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

    //* monitors a file for changes and hot reloads
    struct FileMonitor {
        //: path to the file
        str file;

        //: last modified time of the file
        fs::file_time_type last_modified;

        //: callback to call when the file changes
        std::function<void(str_view)> callback;

        //: constructor
        FileMonitor(str_view fp, std::function<void(str_view)> callback) : callback(callback) {
            file = path(fp);
            last_modified = fs::last_write_time(file);
            FileSystem::add([this]{update();});
        }

        //: update the file monitor
        void update() {
            //: if the last modified time has changed, call the callback
            if (fs::last_write_time(file) != last_modified) {
                last_modified = fs::last_write_time(file);
                callback(file);
            }

            FileSystem::add([this]{update();});
        }
    };

    //* create a file monitor
    inline std::vector<std::unique_ptr<FileMonitor>> monitors;
    inline void monitor(str_view fp, std::function<void(str_view)> callback) {
        monitors.push_back(std::make_unique<FileMonitor>(fp, callback));
    }

    //* create a file monitor only when hot reloading is enabled
    inline void hot_reload(str_view fp, std::function<void(str_view)> callback) {
        if constexpr (engine_config.hot_reload())
            monitor(fp, callback);
    }
}