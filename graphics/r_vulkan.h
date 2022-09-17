//* vulkan
//      contains all the vulkan specific data structures as well as the vulkan loader
#pragma once

#include "r_types.h"
#include "fresa_config.h"

#include <stack>

namespace fresa::graphics
{
    // ·······················
    // · RENDERING DATATYPES ·
    // ·······················

    namespace vk
    {
        //: indices of the queue families
        enum struct QueueIndices {
            GRAPHICS,
            PRESENT,
            TRANSFER,
            COMPUTE,
            COUNT
        };

        //: physical device, representation of a gpu
        //      contains key vulkan structures, such as the gpu (physical device) and the logical device, which are used in almost every function
        //      there is also some important information about the gpu capabilities and extensions, as well as a list of available queues
        struct GPU {
            VkPhysicalDevice gpu = VK_NULL_HANDLE;
            VkDevice device = VK_NULL_HANDLE;

            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceMemoryProperties memory;

            std::array<int, (ui32)QueueIndices::COUNT> queue_indices = {-1, -1, -1, -1};
            std::vector<VkQueue> queues = {};

            int score = -1;
        };

        //: swapchain
        //      it is what we use to present images to the screen
        //      must be recreated when the window changes
        struct Swapchain {
            VkSwapchainKHR swapchain;

            VkFormat format;
            VkExtent2D extent;
            
            ui32 size;
            
            std::vector<VkImageView> image_views;
            std::vector<VkImage> images;
            std::vector<VkFence> fences_images_in_flight;
        };

        //: deletion queue
        //      called in reversed order of insertion to delete all resources without dependency issues
        struct DeletionQueue {
            std::stack<std::function<void()>> queue;
            void push(std::function<void()> &&f) { queue.push(f); }
            void clear() { while (!queue.empty()) { queue.top()(); queue.pop(); } }
        };

        //: per frame data
        //      since we can have multiple frames in flight (for ex. one being worked on by the cpu and one being presented by the gpu)
        //      we need to have some resources that are unique to each frame, such as command pools and syncronization objects
        struct FrameData {
            VkCommandPool command_pool = VK_NULL_HANDLE;
            VkCommandBuffer main_command_buffer = VK_NULL_HANDLE;

            VkSemaphore image_available_semaphore;
            VkSemaphore render_finished_semaphore;
            VkFence fence_in_flight;

            DeletionQueue deletion_queue_frame;
        };
    }

    // ··············
    // · VULKAN API ·
    // ··············

    //: vulkan data
    struct VulkanAPI {
        //: initialization structures
        VkInstance instance;
        vk::GPU gpu;
        
        //: memory allocator (using vma)
        VmaAllocator allocator;

        //: presentation
        VkSurfaceKHR surface;
        vk::Swapchain swapchain;

        //: per frame data
        std::array<vk::FrameData, engine_config.vk_frames_in_flight()> frame;

        //: descriptors
        std::vector<VkDescriptorPool> descriptor_pools;

        //: deletion queues
        vk::DeletionQueue deletion_queue_global;
        vk::DeletionQueue deletion_queue_swapchain;

        //: debug
        VkDebugReportCallbackEXT debug_callback;
    };
}