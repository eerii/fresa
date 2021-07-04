//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan.h"
#include "config.h"

#include <optional>

namespace Verse::Graphics
{
    struct Vulkan;

    namespace VK {
        void initVulkan(Vulkan *vulkan, Config &c);

        struct QueueFamilyIndices {
            std::optional<ui32> graphics_queue_family_index;
            std::optional<ui32> present_queue_family_index;
            std::optional<ui32> compute_queue_family_index;
            
            bool all() { return (graphics_queue_family_index.has_value() and present_queue_family_index.has_value() and
                                 compute_queue_family_index.has_value()); };
        };
    }

    struct Vulkan {
        //CORE
        //----------------------------------------
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        void createInstance(Config &c);
        
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        ui16 ratePhysicalDevice(VkPhysicalDevice physical_device);
        void selectPhysicalDevice();
        
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkQueue compute_queue;
        VK::QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physical_device);
        VK::QueueFamilyIndices queue_families;
        void selectQueueFamily();
        
        VkDevice device;
        void createDevice();
        
        VkSurfaceKHR surface;
        void createSurface(Config &c);
        
        VkDebugReportCallbackEXT debug_callback;
        void createDebug();
        //----------------------------------------
        
        //SCREEN
        //----------------------------------------
        VkSwapchainKHR swapchain;
        
        VkSurfaceCapabilitiesKHR surface_capabilities;
        VkSurfaceFormatKHR surface_format;
        VkExtent2D swapchain_size;
        
        std::vector<VkImage> swapchain_images;
        ui32 swapchain_image_count;
        bool createSwapchain(bool resize);
        
        std::vector<VkImageView> swapchain_image_views;
        void createImageViews();
        
        VkFormat depth_format;
        VkImage depth_image;
        VkDeviceMemory depth_image_memory;
        VkImageView depth_image_view;
        void setupDepthStencil();
        
        VkRenderPass render_pass;
        void createRenderPass();

        std::vector<VkFramebuffer> swapchain_framebuffers;
        void createFramebuffers();
        //----------------------------------------
        
        //IMAGES
        //----------------------------------------
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
        void createImage(ui32 width, ui32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);
        //----------------------------------------
        
        //OTHER
        //----------------------------------------
        VkCommandPool command_pool;
        void createCommandPool();

        std::vector<VkCommandBuffer> command_buffers;
        void createCommandBuffers();

        VkSemaphore image_available_semaphore;
        VkSemaphore rendering_finished_semaphore;
        void createSemaphore(VkSemaphore *semaphore);
        void createSemaphores();

        std::vector<VkFence> fences;
        void create_fences();
        
        ui32 findMemoryType(ui32 type_filter, VkMemoryPropertyFlags properties);
        //----------------------------------------
    };
}

#endif
