//* vulkan
//      contains all the vulkan specific structs as well as the vulkan loader
#pragma once

#include <glad/vulkan.h>
#include "std_types.h"

namespace fresa::graphics
{
    namespace vk
    {
        struct GPU {
            VkPhysicalDevice gpu;
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceMemoryProperties memory;
            ui16 score;
        };
    }

    struct VulkanAPI {
        VkInstance instance;
        vk::GPU gpu;

        VkDebugReportCallbackEXT debug_callback;
    };
}