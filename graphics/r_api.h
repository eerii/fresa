//* rendering_api
//      the main entrypoint of the graphics engine
#pragma once

//: glad vulkan loader
#include <glad/vulkan.h>

//: vulkan memory allocator (load funcions dinamically)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

//: glfw windowing library
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//: spirv-cross, shader reflection and cross compilation
#include "spirv_glsl.hpp"
namespace spv_c = spirv_cross;

#include "std_types.h"
#include "fresa_config.h"
#include "system.h"
#include <stack>

namespace fresa::graphics
{
    // ···················
    // · GRAPHICS SYSTEM ·
    // ···················

    struct GraphicsSystem {
        inline static System<GraphicsSystem, system::SystemPriority::GRAPHICS> system;
        //: defined on r_api_*.cpp
        static void init();
        static void update();
        static void stop();
    };

    //: graphics api object
    //      holds information specific to the graphics api currently rendering (vk, gl, ...)
    namespace vk { struct VulkanAPI; }
    using GraphicsAPI = vk::VulkanAPI;
    inline std::unique_ptr<const GraphicsAPI> api;

    // ····················
    // · VULKAN DATATYPES ·
    // ····················

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
            std::vector<VkQueue> queues;

            int score = -1;
        };

        //: swapchain
        //      it is what we use to present images to the screen
        //      must be recreated when the window changes
        struct Swapchain {
            VkSwapchainKHR swapchain = VK_NULL_HANDLE;

            VkFormat format = VK_FORMAT_UNDEFINED;
            VkExtent2D extent = {0, 0};
            
            ui32 size = 0;
            
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

            VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
            VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
            VkFence fence_in_flight = VK_NULL_HANDLE;

            DeletionQueue deletion_queue_frame;
        };

        //: vulkan api
        struct VulkanAPI {
            //: initialization structures
            VkInstance instance = VK_NULL_HANDLE;
            vk::GPU gpu;
            
            //: memory allocator (using vma)
            VmaAllocator allocator = VK_NULL_HANDLE;

            //: presentation
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            vk::Swapchain swapchain;

            //: per frame data
            std::array<vk::FrameData, engine_config.vk_frames_in_flight()> frame;

            //: descriptors
            std::vector<VkDescriptorPool> descriptor_pools;

            //: deletion queues
            vk::DeletionQueue deletion_queue_global;
            vk::DeletionQueue deletion_queue_swapchain;

            //: debug
            VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;
        };
    }
}