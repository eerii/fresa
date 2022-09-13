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
#include "fresa_config.h"

#include <stack>

namespace fresa::graphics
{
    namespace vk
    {
        //* indices of the queue families
        enum QueueIndices {
            QUEUE_INDICES_GRAPHICS,
            QUEUE_INDICES_PRESENT,
            QUEUE_INDICES_TRANSFER,
            QUEUE_INDICES_COMPUTE,
            QUEUE_INDICES_COUNT
        };

        //* physical device, representation of a gpu
        struct GPU {
            VkPhysicalDevice gpu = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;

            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceMemoryProperties memory;

            std::array<int, QUEUE_INDICES_COUNT> queue_indices = {-1, -1, -1, -1};
            std::vector<VkQueue> queues = {};

            int score = -1;
        };

        //* swapchain information, necessary to present to the screen
        struct Swapchain {
            VkSwapchainKHR swapchain;

            VkFormat format;
            VkExtent2D extent;
            
            ui32 size;
            
            std::vector<VkImageView> image_views;
            std::vector<VkImage> images;
            std::vector<VkFence> fences_images_in_flight;
        };

        //* deletion queue
        //      called in reversed order of insertion to delete all resources without dependency issues
        struct DeletionQueue {
            std::stack<std::function<void()>> queue;
            void push(std::function<void()> &&f) { queue.push(f); }
            void clear() { while (!queue.empty()) { queue.top()(); queue.pop(); } }
        };

        //* per frame data
        struct FrameData {
            VkCommandPool command_pool = VK_NULL_HANDLE;
            VkCommandBuffer main_command_buffer = VK_NULL_HANDLE;

            VkSemaphore image_available_semaphore;
            VkSemaphore render_finished_semaphore;
            VkFence fence_in_flight;

            DeletionQueue deletion_queue_frame;
        };
    }

    //* vulkan data
    struct VulkanAPI {
        VkInstance instance;
        vk::GPU gpu;
        
        VmaAllocator allocator;

        VkSurfaceKHR surface;
        vk::Swapchain swapchain;

        std::array<vk::FrameData, engine_config.vk_frames_in_flight()> frame;

        vk::DeletionQueue deletion_queue_global;
        vk::DeletionQueue deletion_queue_swapchain;

        VkDebugReportCallbackEXT debug_callback;
    };
}