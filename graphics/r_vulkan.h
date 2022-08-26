//* vulkan
//      contains all the vulkan specific data structures as well as the vulkan loader
#pragma once

#include <glad/vulkan.h>
#include "std_types.h"

namespace fresa::graphics
{
    namespace vk
    {
        //* physical device, representation of a gpu
        struct GPU {
            VkPhysicalDevice gpu = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceMemoryProperties memory;
            std::array<int, 4> queue_indices = {-1, -1, -1, -1};
            int score = -1;
        };

        //* indices of the queue families
        enum QueueIndices {
            QUEUE_INDICES_GRAPHICS,
            QUEUE_INDICES_PRESENT,
            QUEUE_INDICES_TRANSFER,
            QUEUE_INDICES_COMPUTE
        };
    }

    //* vulkan data
    struct VulkanAPI {
        VkInstance instance;
        VkSurfaceKHR surface;
        vk::GPU gpu;

        VkDebugReportCallbackEXT debug_callback;
    };
}