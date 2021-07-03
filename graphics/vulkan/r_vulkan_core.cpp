//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

namespace Verse::Graphics
{

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    void Vulkan::createInstance(Config &c) {
        ui32 extension_count;
        SDL_Vulkan_GetInstanceExtensions(c.window, &extension_count, nullptr);
        std::vector<const char *> extension_names(extension_count);
        SDL_Vulkan_GetInstanceExtensions(c.window, &extension_count, extension_names.data());
        log::graphics("Vulkan extensions supported: %d", extension_count);
        for (const char* ext : extension_names)
            log::graphics("- %s", ext);
        
        ui32 validation_layer_count;
        vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
        std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
        vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
        log::graphics("Vulkan validation layers supported: %d", validation_layer_count);
        for (const auto &layer : available_validation_layers)
            log::graphics("- %s", layer.layerName);
        
        for (const char* val : validation_layers) {
            bool layer_exists = false;
            for (const auto &layer : available_validation_layers) {
                if (str(val) == str(layer.layerName)) {
                    layer_exists = true;
                    break;
                }
            }
            if (not layer_exists) {
                log::error("Attempted to use a validation layer but it is not supported (%s)", val);
                SDL_Quit();
            }
        }
        
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = c.name.c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(c.version[0], c.version[1], c.version[2]);
        app_info.pEngineName = "Fresa";
        app_info.engineVersion = VK_MAKE_VERSION(c.version[0], c.version[1], c.version[2]);
        app_info.apiVersion = VK_API_VERSION_1_1;
        
        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;
        instance_create_info.enabledLayerCount = (int)validation_layers.size();
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
        instance_create_info.enabledExtensionCount = (int)extension_names.size();
        instance_create_info.ppEnabledExtensionNames = extension_names.data();
        
        vkCreateInstance(&instance_create_info, nullptr, &instance);
    }

}

#endif
