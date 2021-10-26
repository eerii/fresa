//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

VK::SwapchainSupportDetails Vulkan::getSwapchainSupport(VkPhysicalDevice physical_device) {
    VK::SwapchainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
    
    ui32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
    }
    
    ui32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data());
    }
    
    return details;
}

VkSurfaceFormatKHR Vulkan::selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (VkSurfaceFormatKHR fmt : available_formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return fmt;
    }
    return available_formats[0];
}

VkPresentModeKHR Vulkan::selectSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes) {
    //Fifo: Vsync, when the queue is full the program waits
    //Mailbox: Triple buffering, the program replaces the last images of the queue, less latency but more power consumption
    
    for (VkPresentModeKHR mode : available_present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            log::graphics("Present Mode: Mailbox");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    
    log::graphics("Present Mode: Fifo");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::selectSwapExtent(WindowData &win, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(win.window, &w, &h);
    
    VkExtent2D actual_extent{ static_cast<ui32>(w), static_cast<ui32>(h) };
    
    std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actual_extent;
}

void Vulkan::createSwapchain(WindowData &win) {
    VK::SwapchainSupportDetails swapchain_support = getSwapchainSupport(physical_device);
    
    VkSurfaceFormatKHR surface_format = selectSwapSurfaceFormat(swapchain_support.formats);
    VkPresentModeKHR present_mode = selectSwapPresentMode(swapchain_support.present_modes);
    swapchain_extent = selectSwapExtent(win, swapchain_support.capabilities);
    swapchain_format = surface_format.format;
    
    ui32 image_count = swapchain_support.capabilities.minImageCount + 1;
    
    if (swapchain_support.capabilities.maxImageCount > 0 and image_count > swapchain_support.capabilities.maxImageCount)
        image_count = swapchain_support.capabilities.maxImageCount;
    
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swapchain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    ui32 queue_family_indices[2]{
        queue_families.graphics_queue_family_index.value(),
        queue_families.present_queue_family_index.value()
    };
    
    if (queue_family_indices[0] != queue_family_indices[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE; //TODO: Allow recreation of swapchains
    
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain)!= VK_SUCCESS)
        log::error("Error creating the Vulkan Swapchain");
    
    log::graphics("Created the Vulkan Swapchain");
    
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());
}

void Vulkan::createImageViews() {
    swapchain_image_views.resize(swapchain_images.size());
    
    for (int i = 0; i < swapchain_images.size(); i++)
        swapchain_image_views[i] = createImageView(swapchain_images[i], VK_IMAGE_ASPECT_COLOR_BIT);
    
    log::graphics("Created all Vulkan Image Views");
}

VkImageView Vulkan::createImageView(VkImage image, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo create_info{};
    
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = swapchain_format;
    
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    
    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkImageView image_view;
    if (vkCreateImageView(device, &create_info, nullptr, &image_view)!= VK_SUCCESS)
        log::error("Error creating a Vulkan Image View");
    
    return image_view;
}

void Vulkan::recreateSwapchain(WindowData &win) {
    vkDeviceWaitIdle(device);
    
    destroySwapchain();
    
    createSwapchain(win);
    createImageViews();
    
    createRenderPass();
    createGraphicsPipeline();
    
    createFramebuffers();
    
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    
    createCommandBuffers();
}

#endif
