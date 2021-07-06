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
    
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };
    }

    struct Vulkan {
        //DEVICE
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
        
        //SWAPCHAIN
        //----------------------------------------
        VK::SwapchainSupportDetails getSwapchainSupport(VkPhysicalDevice physical_device);
        VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D selectSwapExtent(Config &c, const VkSurfaceCapabilitiesKHR& capabilities);
        
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
        
        VkSwapchainKHR swapchain;
        void createSwapchain(Config &c);
        
        std::vector<VkImageView> swapchain_image_views;
        void createImageViews();
        VkImageView createImageView(VkImage image, VkImageAspectFlags aspect_flags);
        //----------------------------------------
        
        //PIPELINE
        //----------------------------------------
        void createGraphicsPipeline();
        //----------------------------------------
        
        //CLEANUP
        //----------------------------------------
        void destroy();
        //----------------------------------------
    };
}

#endif
