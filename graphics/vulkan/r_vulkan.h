//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>

#include "r_vulkan_datatypes.h"

namespace Verse::Graphics
{
    struct Vulkan {
        //Device
        //----------------------------------------
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        
        VkSurfaceKHR surface;
        
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        
        VK::QueueData queues;
        
        VkDevice device;
        //----------------------------------------
        
        
        //Swapchain
        //----------------------------------------
        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
        
        VkSwapchainKHR swapchain;
        
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;
        //----------------------------------------
        
        
        //Debug
        //----------------------------------------
        VkDebugReportCallbackEXT debug_callback;
        //----------------------------------------
    };
}


#endif


