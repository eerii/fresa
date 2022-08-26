//* vulkan
//      contains all the vulkan specific data structures as well as the vulkan loader
#pragma once

//: glad vulkan loader
#include <glad/vulkan.h>

//: vulkan memory allocator (load funcions dinamically)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

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
            std::vector<VkQueue> queues = {};

            int score = -1;
        };

        //* indices of the queue families
        enum QueueIndices {
            QUEUE_INDICES_GRAPHICS,
            QUEUE_INDICES_PRESENT,
            QUEUE_INDICES_TRANSFER,
            QUEUE_INDICES_COMPUTE
        };

        //* swapchain information, necessary to present to the screen
        struct Swapchain {
            VkSwapchainKHR swapchain;

            VkFormat format;
            VkExtent2D extent;
            
            ui32 size;
            
            std::vector<VkImageView> image_views;
            std::vector<VkImage> images;
        };
    }

    //* vulkan data
    struct VulkanAPI {
        VkInstance instance;
        vk::GPU gpu;
        
        VmaAllocator allocator;

        VkSurfaceKHR surface;
        vk::Swapchain swapchain;

        VkDebugReportCallbackEXT debug_callback;
    };
}