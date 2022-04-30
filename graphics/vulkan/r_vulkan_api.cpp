//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#ifdef USE_VULKAN

#define VMA_IMPLEMENTATION
#ifndef _MSC_VER
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Weverything"
    #include "vk_mem_alloc.h"
    #pragma clang diagnostic pop
#else
    #include "vk_mem_alloc.h"
#endif

#include "r_vulkan_api.h"
#include "r_window.h"
#include "r_camera.h"

#include "gui.h"
#include "f_time.h"

#include <set>
#include <numeric>
#include <fstream>

using namespace Fresa;
using namespace Graphics;

namespace {
    const std::vector<const char*> validation_layers{
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> required_device_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
}



//Initialization
//----------------------------------------

void Graphics::configureAPI() {
    
}

void Graphics::createAPI() {
    //---Instance---
    api.instance = VK::createInstance();
    api.debug_callback = VK::createDebug(api.instance);
    
    //---Surface---
    api.surface = VK::createSurface(api.instance);
    
    //---Physical device---
    api.physical_device = VK::selectPhysicalDevice(api.instance, api.surface, api.physical_device_features, api.physical_device_properties);
    
    //---Queues and logical device---
    api.cmd.queue_indices = VK::getQueueFamilies(api.surface, api.physical_device);
    api.device = VK::createDevice(api.physical_device, api.physical_device_features, api.cmd.queue_indices);
    api.cmd.queues = VK::getQueues(api.device, api.cmd.queue_indices);
    
    //---Allocator---
    api.allocator = VK::createMemoryAllocator(api.device, api.physical_device, api.instance);
    
    //---Swapchain---
    api.swapchain = VK::createSwapchain(api.device, api.physical_device, api.surface, api.cmd.queue_indices);
    
    //---Command pools---
    api.cmd.command_pools = VK::createCommandPools(api.device, api.cmd.queue_indices,
    {   {"draw",     {api.cmd.queue_indices.present.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT}},
        {"temp",     {api.cmd.queue_indices.graphics.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT}},
        {"transfer", {api.cmd.queue_indices.transfer.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT}},
        {"compute",  {api.cmd.queue_indices.compute.value(), VkCommandPoolCreateFlagBits(0)}},
    });
    api.cmd.command_buffers["draw"] = VK::allocateDrawCommandBuffers();
    
    //---Sync objects---
    api.sync = VK::createSyncObjects();
    
    //---Image sampler---
    api.sampler = VK::createSampler(api.device);
    
    //---Render passes---
    processRendererDescription();
    
    //---Compute pipelines---
    //TODO: ENABLE COMPUTE SHADERS
    log::warn("Enable compute shaders");
    /*for (auto &[shader, data] : compute_shaders)
        api.compute_pipelines[shader] = VK::createComputePipeline(api, shader);*/
    
    //---Window vertex buffer---
    api.window_vertex_buffer = VK::createVertexBuffer(Vertices::window);
}

//----------------------------------------



//Device
//----------------------------------------

VkInstance VK::createInstance() {
    log::graphics("");
    
    //---Instance extensions---
    //      Add extra functionality to Vulkan
    ui32 extension_count;
    SDL_Vulkan_GetInstanceExtensions(window.window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(window.window, &extension_count, extension_names.data());
    log::graphics("Vulkan requested instance extensions: %d", extension_count);
    for (const auto &ext : extension_names)
        log::graphics(" - %s", ext);
    
    log::graphics("");
    
    //---Validation layers---
    //      Middleware for existing Vulkan functionality
    //      Primarily used for getting detailed error descriptions, in this case with VK_LAYER_KHRONOS_validation
    //      Disabled when not using DEBUG mode
    ui32 validation_layer_count;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
    log::graphics("Vulkan supported validation layers: %d", validation_layer_count);
    for (VkLayerProperties layer : available_validation_layers)
        log::graphics(" - %s", layer.layerName);
    
    log::graphics("");
    
    #ifdef DEBUG
    for (const auto &val : validation_layers) {
        bool layer_exists = false;
        for (const auto &layer : available_validation_layers) {
            if (str(val) == str(layer.layerName)) {
                layer_exists = true;
                break;
            }
        }
        if (not layer_exists)
            log::warn("Attempted to use a validation layer but it is not supported (%s)", val);
    }
    #endif
    
    //---App info---
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = Config::name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(Config::version[0], Config::version[1], Config::version[2]);
    app_info.pEngineName = "Fresa";
    app_info.engineVersion = VK_MAKE_VERSION(Config::version[0], Config::version[1], Config::version[2]);
    app_info.apiVersion = VK_API_VERSION_1_1;
    
    //---Instance create info---
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = (int)extension_names.size();
    instance_create_info.ppEnabledExtensionNames = extension_names.data();
    
    #ifdef DEBUG
    instance_create_info.enabledLayerCount = (int)validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
    #endif
    
    //---Create instance---
    VkInstance instance;
    
    if (vkCreateInstance(&instance_create_info, nullptr, &instance)!= VK_SUCCESS)
        log::error("Fatal error creating a vulkan instance");
    
    deletion_queue_program.push_back([instance](){vkDestroyInstance(instance, nullptr);});
    return instance;
}

VkSurfaceKHR VK::createSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    
    //---Surface---
    //      It is the abstraction of the window created by SDL to something Vulkan can draw onto
    if (not SDL_Vulkan_CreateSurface(window.window, instance, &surface))
        log::error("Fatal error while creating a vulkan surface (from createSurface): %s", SDL_GetError());
    
    deletion_queue_program.push_back([surface, instance](){vkDestroySurfaceKHR(instance, surface, nullptr);});
    return surface;
}

ui16 VK::ratePhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    ui16 score = 16;
    
    //---Device properties---
    //      Holds information about the GPU, such as if it is discrete or integrated
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    
    //: Prefer a discrete GPU
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 256;
    
    
    //---Features---
    //      What capabilites are supported with this device
    //VkPhysicalDeviceFeatures device_features = getPhysicalDeviceFeatures(physical_device);
    
    //: (optional) Anisotropy
    //      Helps when rendering with perspective
    //      Provides a higher quality at the cost of performance
    //      Not all devices suppoport it, so it has to be checked for and enabled
    //  if (not device_features.samplerAnisotropy) return 0;
    
    
    //---Queues---
    //      Different execution ports of the GPU, command buffers are submitted here
    //      There are different spetialized queue families, such as present and graphics
    VkQueueIndices queue_indices = getQueueFamilies(surface, physical_device);
    if (queue_indices.compute.has_value())
        score += 16;
    if (not queue_indices.present.has_value())
        return 0;
    if (not queue_indices.graphics.has_value())
        return 0;
    
    
    //---Extensions---
    //      Not everything is core in Vulkan, so we need to enable some extensions
    //      Here we check for required extensions (defined up top) and choose physical devices that support them
    //      The main one is vkSwapchainKHR, the surface we draw onto, which is implementation dependent
    ui32 extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());
    
    std::set<std::string> required_extensions(required_device_extensions.begin(), required_device_extensions.end());
    
    for (const VkExtensionProperties &extension : available_extensions)
        required_extensions.erase(extension.extensionName);

    if (not required_extensions.empty())
        return 0;
    
    //---Swapchain---
    //      Make sure that the device supports swapchain presentation
    VkSwapchainSupportData swapchain_support = VK::getSwapchainSupport(surface, physical_device);
    if (swapchain_support.formats.empty() or swapchain_support.present_modes.empty())
        return 0;
    
    return score;
}

VkPhysicalDevice VK::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
                                          VkPhysicalDeviceFeatures &features, VkPhysicalDeviceProperties &properties) {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    
    //---Show requested device extensions---
    log::graphics("Vulkan required device extensions: %d", required_device_extensions.size());
    for (const auto &ext : required_device_extensions)
        log::graphics(" - %s", ext);
    log::graphics("");
    
    
    //---Get available physical devices---
    ui32 device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0)
        log::error("There are no GPUs with vulkan support!");
    
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    log::graphics("Vulkan physical devices: %d", device_count);
    
    
    //---Rate physical devices---
    //      Using ratePhysicalDevice(), assign a number to each available GPU
    //      Highest number gets chosen as the main physical device for the program
    //      It is possible to use multiple GPUs, but we won't support it yet
    std::multimap<VkPhysicalDevice, ui16> candidates;
    for (VkPhysicalDevice device : devices)
        candidates.insert(std::make_pair(device, ratePhysicalDevice(surface, device)));
    auto chosen = std::max_element(candidates.begin(), candidates.end(), [](auto &a, auto &b){ return a.second < b.second;});
    if (chosen->second > 0)
        physical_device = chosen->first;
    
    if (physical_device == VK_NULL_HANDLE)
        log::error("No GPU passed the vulkan physical device requirements.");
    
    //: Get physical device features
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    
    //: Get physical device properties
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    
    //---Show the result of the process---
    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        log::graphics(((device == physical_device) ? " > %s" : " - %s"), device_properties.deviceName);
    }
    log::graphics("");
    
    
    
    return physical_device;
}

VkFormat VK::chooseSupportedFormat(VkPhysicalDevice physical_device, const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling, VkFormatFeatureFlags features) {
    //---Choose supported format---
    //      From a list of candidate formats in order of preference, select a supported VkFormat
    
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
        
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    
    log::error("Failed to find a suitable supported format");
    return candidates[0];
}

VkSampleCountFlagBits VK::getMaxMSAASamples(VkPhysicalDeviceProperties &properties) {
    VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
    
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

VkQueueIndices VK::getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    //---Queues---
    //      Different execution ports of the GPU, command buffers are submitted here
    //      There are different spetialized queue families, such as present, graphics and compute
    
    
    //---Get all available queues---
    ui32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_list.data());
    
    
    //---Select desired queues---
    //      Using the VkQueueIndices struct, which has 3 std::optional indices, for:
    //          - Graphics: pipeline operations, including vertex/fragment shaders and drawing
    //          - Present: send framebuffers to the screen
    //          - Compute: for compute shaders
    //      Not all queues are needed, and in the future more queues can be created for multithread support
    //      Made so present and graphics queue can be the same
    VkQueueIndices queue_indices;
    for (int i = 0; i < queue_family_list.size(); i++) {
        if (not queue_indices.present.has_value()) {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
            if(present_support)
                queue_indices.present = i;
        }
        
        if (not queue_indices.graphics.has_value() and (queue_family_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            queue_indices.graphics = i;
            continue;
        }
        
        if (not queue_indices.compute.has_value() and (queue_family_list[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            queue_indices.compute = i;
            continue;
        }
        
        if (not queue_indices.transfer.has_value() and (queue_family_list[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            queue_indices.transfer = i;
            continue;
        }
        
        if (queue_indices.graphics.has_value() and queue_indices.present.has_value() and queue_indices.compute.has_value())
            break;
    }
    
    if (not queue_indices.graphics.has_value())
        log::error("No graphics queue was found! This is terrible aaaa :(((");
    
    if (not queue_indices.transfer.has_value()) {
        log::warn("No transfer specific queue was found, using the graphics queue as the default");
        queue_indices.transfer = queue_indices.graphics.value();
    }
    
    return queue_indices;
}

VkDevice VK::createDevice(VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures physical_device_features,
                          const VkQueueIndices &queue_indices) {
    //---Logical device---
    //      Vulkan GPU driver, it encapsulates the physical device
    //      Needs to be passed to almost every Vulkan function
    VkDevice device;
    
    //---Create the selected queues---
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    
    std::set<ui32> unique_queue_families{};
    if (queue_indices.graphics.has_value())
        unique_queue_families.insert(queue_indices.graphics.value());
    if (queue_indices.present.has_value())
        unique_queue_families.insert(queue_indices.present.value());
    if (queue_indices.compute.has_value())
        unique_queue_families.insert(queue_indices.compute.value());
    if (queue_indices.transfer.has_value())
        unique_queue_families.insert(queue_indices.transfer.value());
    log::graphics("Vulkan queue families: %d", unique_queue_families.size());
    
    int i = 0;
    std::vector<float> priorities{ 1.0f, 1.0f, 0.5f, 0.5f };
    
    for (ui32 family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = family;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &priorities[i];
        queue_create_infos.push_back(queue_info);
        i++;
    }
    
    //---Device required features---
    //      Enable some features here, try to keep it as small as possible
    VkPhysicalDeviceFeatures enabled_features{};
    if (physical_device_features.fillModeNonSolid) //: Wireframe mode
        enabled_features.fillModeNonSolid = VK_TRUE;
    if (physical_device_features.wideLines) //: Lines with width different than 1.0f
        enabled_features.wideLines = VK_TRUE;
    
    //---Create device---
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    //: Add the queues we selected before to be created
    device_create_info.queueCreateInfoCount = (int)queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = (int)required_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.pEnabledFeatures = &enabled_features;
    
    device_create_info.enabledLayerCount = (int)validation_layers.size();
    device_create_info.ppEnabledLayerNames = validation_layers.data();
    
    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device)!= VK_SUCCESS)
        log::error("Error creating a vulkan logical device");
    
    deletion_queue_program.push_back([device](){
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device, nullptr);
    });
    return device;
}

VkQueueData VK::getQueues(VkDevice device, const VkQueueIndices &queue_indices) {
    VkQueueData queues;
    
    if (queue_indices.graphics.has_value()) {
        vkGetDeviceQueue(device, queue_indices.graphics.value(), 0, &queues.graphics);
        log::graphics(" - Graphics (%d)", queue_indices.graphics.value());
    }
    if (queue_indices.present.has_value()) {
        vkGetDeviceQueue(device, queue_indices.present.value(), 0, &queues.present);
        log::graphics(" - Present (%d)", queue_indices.present.value());
    }
    if (queue_indices.compute.has_value()) {
        vkGetDeviceQueue(device, queue_indices.compute.value(), 0, &queues.compute);
        log::graphics(" - Compute (%d)", queue_indices.compute.value());
    }
    if (queue_indices.transfer.has_value()) {
        vkGetDeviceQueue(device, queue_indices.transfer.value(), 0, &queues.transfer);
        log::graphics(" - Transfer (%d)", queue_indices.transfer.value());
    }
    log::graphics("");
    
    return queues;
}

//----------------------------------------



//Memory
//----------------------------------------

VmaAllocator VK::createMemoryAllocator(VkDevice device, VkPhysicalDevice physical_device, VkInstance instance) {
    //---Memory allocation---
    //      Memory management in vulkan is really tedious, since there are many memory types (CPU, GPU...) with different limitations and speeds
    //      Buffers and images have to be accompained by a VkDeviceMemory which needs to be allocated by vkAllocateMemory
    //      The problem is that it is discouraged to call vkAllocateMemory per buffer, since the number of allowed allocations is small
    //          even on top tier hardware, for example, 4096 on a GTX 1080
    //      The solution is to allocate big chunks of memory and then suballocate to each resource, using offsets and keeping track of
    //          where each buffer resides and how big it is
    //      This is hard to do right while avoiding fragmentation and overlaps, so we are integrating the Vulkan Memory Allocator library,
    //          which takes care of the big chunk allocation and position for us. It is possible to write a smaller tool to help, but in an
    //          attempt to keep the scope of the project manageable (says the one writing vulkan for a 2d tiny engine...) we'll leave it for now
    //      Here we are creating the VmaAllocator object, which we will have to reference during buffer creation and will house the pools of
    //          memory that we will be using
    VmaAllocator allocator;
    
    VmaAllocatorCreateInfo create_info{};
    create_info.vulkanApiVersion = VK_API_VERSION_1_1;
    create_info.device = device;
    create_info.physicalDevice = physical_device;
    create_info.instance = instance;
    
    if (vmaCreateAllocator(&create_info, &allocator) != VK_SUCCESS)
        log::error("Error creating the vulkan memory allocator");
    
    deletion_queue_program.push_back([allocator](){vmaDestroyAllocator(allocator);});
    return allocator;
}

//----------------------------------------



//Swapchain
//----------------------------------------

VkSwapchainSupportData VK::getSwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    VkSwapchainSupportData swapchain_support;
    
    //---Surface capabilities---
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &swapchain_support.capabilities);
    
    //---Surface formats---
    ui32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (format_count != 0) {
        swapchain_support.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, swapchain_support.formats.data());
    }
    
    //---Surface present modes---
    ui32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        swapchain_support.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, swapchain_support.present_modes.data());
    }
    
    return swapchain_support;
}

VkSurfaceFormatKHR VK::selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    //---Surface format---
    //      The internal format for the vulkan surface
    //      It might seem weird to use BGRA instead of RGBA, but displays usually use this pixel data format instead
    //      Vulkan automatically converts our framebuffers to this space so we don't need to worry
    //      We should use an SRGB format, but we will stick with UNORM for now for testing purposes
    //      If all fails, it will still select a format, but it might not be the perfect one
    for (const VkSurfaceFormatKHR &fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return fmt;
    }
    log::warn("A non ideal format has been selected for the swap surface, since BGRA UNORM is not supported. You might experience that the graphics present in unexpected colors. Please check the GPU support for ideal representation.");
    return formats[0];
}

VkPresentModeKHR VK::selectSwapPresentMode(const std::vector<VkPresentModeKHR> &modes) {
    //---Surface present mode---
    //      The way the buffers are swaped to the screen
    //      - Fifo: Vsync, when the queue is full the program waits
    //      - Mailbox: Triple buffering, the program replaces the last images of the queue, less latency but more power consumption
    //      Not all GPUs support mailbox (for example integrated Intel GPUs), so while it is preferred, Fifo can be used as well
    
    if (Config::prefer_mailbox_mode) {
        for (const VkPresentModeKHR &mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                log::graphics("Present mode: Mailbox");
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
    }
    
    log::graphics("Present mode: Fifo");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VK::selectSwapExtent(VkSurfaceCapabilitiesKHR capabilities) {
    //---Surface extent---
    //      This is the drawable are on the screen
    //      If the current extent is UINT32_MAX, we should calculate the actual extent using WindowData
    //      and clamp it to the min and max supported extent by the GPU
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(window.window, &w, &h);
    
    VkExtent2D actual_extent{ (ui32)w, (ui32)h };
    
    std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actual_extent;
}

SwapchainData VK::createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                  const VkQueueIndices &queue_indices) {
    //---Swapchain---
    //      List of images that will get drawn to the screen by the render pipeline
    //      Swapchain data:
    //      - VkFormat format: Image format of the swapchain, usually B8G8R8A8_SRGB or B8G8R8A8_UNORM
    //      - VkExtent2D extent: The size of the draw area
    //      - VkSwapchainKHR swapchain: Actual swapchain object
    //      - std::vector<VkImage> images: List of swapchain images
    //      - std::vector<VkImageView> image_views: List of their respective image views
    SwapchainData swapchain;
    
    //---Format, present mode and extent---
    VkSwapchainSupportData support = getSwapchainSupport(surface, physical_device);
    VkSurfaceFormatKHR surface_format = selectSwapSurfaceFormat(support.formats);
    swapchain.format = surface_format.format;
    VkPresentModeKHR present_mode = selectSwapPresentMode(support.present_modes);
    swapchain.extent = selectSwapExtent(support.capabilities);
    
    
    //---Number of images---
    swapchain.min_image_count = support.capabilities.minImageCount;
    
    //---Create swapchain---
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    
    //: Swapchain images
    create_info.minImageCount = swapchain.min_image_count + 1;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swapchain.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    //: Specify the sharing mode for the queues
    //  If the graphics and present are the same queue, it must be concurrent
    std::array<ui32,2> queue_family_indices{ queue_indices.graphics.value(), queue_indices.present.value() };
    if (queue_family_indices[0] != queue_family_indices[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    //: Other properties
    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    
    //: Swapchain recreation, disabled right now
    create_info.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain.swapchain)!= VK_SUCCESS)
        log::error("Error creating a vulkan swapchain");
    
    //---Swapchain images---
    vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchain.size, nullptr);
    swapchain.images.resize(swapchain.size);
    vkGetSwapchainImagesKHR(device, swapchain.swapchain, &swapchain.size, swapchain.images.data());
    
    //---Swapchain image views---
    swapchain.image_views.resize(swapchain.size);
    for (int i = 0; i < swapchain.images.size(); i++)
        swapchain.image_views[i] = createImageView(device, swapchain.images[i], VK_IMAGE_ASPECT_COLOR_BIT, swapchain.format);
    
    //---Deletion queue---
    deletion_queue_swapchain.push_back([swapchain](){
        for (VkImageView view : swapchain.image_views)
            vkDestroyImageView(api.device, view, nullptr);
        vkDestroySwapchainKHR(api.device, swapchain.swapchain, nullptr);
    });
    
    log::graphics("Created a vulkan swapchain");
    return swapchain;
}

void VK::recreateSwapchain() {
    //---Recreate swapchain---
    //      When the swapchain is no longer actual, for example on window resize, we need to recreate it
    //      This begins by waiting for draw operations to finish, and then cleaning the resources that depend on the swapchain
    //      After recreating the swapchain, we can check if its size changed
    //      At the current stage this is very unlikely, but if it happened, we should recreate all the objects that depend on this size
    //      We can use deletion_queue_size_change to delete the resources and then allocate them again
    log::graphics("");
    log::graphics("Recreating swapchain...");
    log::graphics("---");
    log::graphics("");
    
    //: Wait for draw operations to finish
    vkDeviceWaitIdle(api.device);
    
    //: Clean previous swapchain
    for (auto it = deletion_queue_swapchain.rbegin(); it != deletion_queue_swapchain.rend(); ++it)
        (*it)();
    deletion_queue_swapchain.clear();
    
    //---Swapchain---
    api.swapchain = createSwapchain(api.device, api.physical_device, api.surface, api.cmd.queue_indices);
    
    //: Attachments
    recreateAttachments();
    for (auto &[id, shader] : shaders.list.at(SHADER_DRAW))
        VK::linkPipelineDescriptors(id);
    for (auto &[id, shader] : shaders.list.at(SHADER_POST))
        VK::linkPipelineDescriptors(id);
    
    //: Render passes and framebuffers
    VK::recreateRenderPasses(api);
}

//----------------------------------------



//Commands
//----------------------------------------

std::map<str, VkCommandPool> VK::createCommandPools(VkDevice device, const VkQueueIndices &queue_indices,
                                                    std::map<str, VkCommandPoolHelperData> create_data) {
    //---Command pools---
    //      Command buffers can be allocated inside them
    //      We can have multiple command pools for different types of buffers, for example, "draw" and "transfer"
    //      Resetting the command pool is easier than individually resetting buffers
    
    std::map<str, VkCommandPool> command_pools;
    
    for (auto &[key, data] : create_data) {
        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        
        //: Queue index for this specific command pool
        create_info.queueFamilyIndex = data.queue.has_value() ? data.queue.value() : queue_indices.graphics.value();
        
        //: Flags, for example, a transient flag for temporary buffers
        create_info.flags = data.flags.has_value() ? data.flags.value() : (VkCommandPoolCreateFlagBits)0;
        
        //: Create the command pool
        if (vkCreateCommandPool(device, &create_info, nullptr, &command_pools[key]) != VK_SUCCESS)
            log::error("Failed to create a vulkan command pool (%s)", key.c_str());
    }
    
    deletion_queue_program.push_back([command_pools](){
        for (auto [key, pool] : command_pools)
            vkDestroyCommandPool(api.device, pool, nullptr);
    });
    log::graphics("Created all vulkan command pools");
    return command_pools;
}

std::array<VkCommandBuffer, Config::frames_in_flight> VK::allocateDrawCommandBuffers() {
    //---Command buffers---
    //      All vulkan commands must be executed inside a command buffer
    //      Here we create the command buffers we will use for drawing, and allocate them inside a command pool ("draw")
    //      We are creating one buffer per swapchain image
    
    std::array<VkCommandBuffer, Config::frames_in_flight> command_buffers{};
    
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = api.cmd.command_pools.at("draw");
    allocate_info.commandBufferCount = (ui32)Config::frames_in_flight;
    
    if (vkAllocateCommandBuffers(api.device, &allocate_info, command_buffers.data()) != VK_SUCCESS)
        log::error("Failed to allocate the vulkan main draw command buffers");
    
    log::graphics("Allocated the vulkan draw command buffers");
    
    deletion_queue_program.push_back([command_buffers](){
        vkFreeCommandBuffers(api.device, api.cmd.command_pools.at("draw"), (ui32)Config::frames_in_flight, command_buffers.data());
    });
    
    return command_buffers;
}

void VK::recordRenderCommandBuffer() {
    //---Draw command buffer---
    //      We are going to use a command buffer for drawing
    //      It needs to bind the vertex and index buffers, as well as the descriptor sets that map the shader inputs such as uniforms
    //      We also have helper functions for begin and end the drawing buffer so it is easier to create different drawings
    const VkCommandBuffer &cmd = api.cmd.command_buffers.at("draw").at(api.sync.current_frame);
    
    //: Begin buffer
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        log::error("Failed to begin recording on a vulkan command buffer");
    
    for (const auto &[r_id, render] : api.render_passes) {
        IF_GUI(if (r_id == api.gui_render_pass) continue;) //: Skip gui render pass
        
        auto subpasses = getAtoB<SubpassID>(r_id, Map::renderpass_subpass);
        auto attachments = getAtoB_v(r_id, Map::renderpass_attachment, api.attachments);
        
        //: Clear values
        std::vector<VkClearValue> clear_values;
        for (auto &[idx, a] : attachments) {
            VkClearValue clear;
            if (a.type & ATTACHMENT_COLOR)
                clear.color = {0.f, 0.f, 0.f, 1.0f};
            if (a.type & ATTACHMENT_DEPTH)
                clear.depthStencil = {1.0f, 0};
            clear_values.push_back(clear);
            if (a.type & ATTACHMENT_MSAA)
                clear_values.push_back(clear);
        }
        
        //: Begin render pass
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render.render_pass;
        render_pass_info.framebuffer = render.framebuffers.at(api.cmd.current_image);
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = render.attachment_extent;
        render_pass_info.clearValueCount = (ui32)clear_values.size();
        render_pass_info.pClearValues = clear_values.data();
        
        vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        
        //: Set viewport and scissor
        setViewport(cmd, render.attachment_extent);
        
        for (const auto &s_id : subpasses) {
            //: Next subpass
            if (s_id != subpasses.at(0))
                vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
            
            //: Get all shaders associated with this subpass
            std::vector<ShaderID> subpass_shaders = getAtoB<ShaderID>(s_id, Map::subpass_shader);
            
            for (const auto &shader_id : subpass_shaders) {
                const ShaderPass& shader = Shader::getShader(shader_id);
                //---Draw shaders---
                if (shaders.types.at(shader_id) == SHADER_DRAW) {
                    if (not api.draw_queue_instanced.count(shader_id))
                        continue;
                    
                    //: Bind pipeline
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);
                    
                    //---Instanced rendering queue---
                    auto queue_uniform = api.draw_queue_instanced.at(shader_id);
                    for (const auto &[uniform_id, queue_geometry] : queue_uniform) {
                        DrawUniformData &uniform = api.draw_uniform_data.at(uniform_id);
                        //: Descriptor set
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline_layout, 0, 1,
                                                &shader.descriptors.at(0).descriptors.at(api.sync.current_frame), 0, nullptr);
                        
                        for (const auto &[geometry_id, queue_instance] : queue_geometry) {
                            GeometryBufferData &geometry = api.geometry_buffer_data.at(geometry_id);
                            
                            VkDeviceSize offsets[]{ 0 };
                            //: Vertex buffer geometry
                            vkCmdBindVertexBuffers(cmd, 0, 1, &geometry.vertex_buffer.buffer, offsets);
                            
                            //: Index buffer
                            VkIndexType index_type = VK_INDEX_TYPE_UINT16;
                            if (geometry.index_bytes == 4) index_type = VK_INDEX_TYPE_UINT32;
                            else if (geometry.index_bytes != 2) log::error("Unsupported index byte size %d", geometry.index_bytes);
                            vkCmdBindIndexBuffer(cmd, geometry.index_buffer.buffer, 0, index_type);
                            
                            for (const auto &[instance_id, description] : queue_instance) {
                                InstancedBufferData &instance = api.instanced_buffer_data.at(instance_id);
                                
                                //: Vertex buffer per instance
                                vkCmdBindVertexBuffers(cmd, 1, 1, &instance.instance_buffer.buffer, offsets);
                                
                                //: Draw indirect
                                if (Config::draw_indirect) {
                                    vkCmdDrawIndexedIndirect(cmd, api.draw_indirect_buffers.at(description->indirect_buffer).buffer.buffer,
                                                             description->indirect_offset * sizeof(VkDrawIndexedIndirectCommand),
                                                             1, sizeof(VkDrawIndexedIndirectCommand));
                                    // - See how multi draw indirect could be implemented, probably you have to squash all the vertex buffers
                                    //   and make use of the vertex offset in the indirect command
                                }
                                //: Draw direct
                                else {
                                    vkCmdDrawIndexed(cmd, geometry.index_size, instance.instance_count, 0, 0, 0);
                                }
                            }
                        }
                    }
                }
                
                //---Post shaders---
                else if (shaders.types.at(shader_id) == SHADER_POST) {
                    //: Pipeline
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);
                    
                    //: Descriptor sets
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline_layout, 0, 1,
                                            &shader.descriptors.at(0).descriptors.at(api.sync.current_frame), 0, nullptr);
                    
                    //: Vertex buffer (It has the 4 vertices of the window area)
                    VkDeviceSize offsets[]{ 0 };
                    vkCmdBindVertexBuffers(cmd, 0, 1, &api.window_vertex_buffer.buffer, offsets);
                    
                    //: Draw
                    vkCmdDraw(cmd, 6, 1, 0, 0);
                }
                
                else {
                    log::error("Shader type for %s is invalid", shader_id.c_str());
                }
            }
        }
        
        //: End render pass
        vkCmdEndRenderPass(cmd);
    }
    
    //: End command buffer
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        log::error("Failed to end recording on a vulkan command buffer");
}

void VK::setViewport(const VkCommandBuffer &cmd, VkExtent2D extent) {
    VkViewport viewport;
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = (float)extent.width; viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdSetDepthBias(cmd, 0, 0, 0);
}

IndirectBufferID Graphics::registerIndirectCommandBuffer() {
    static IndirectBufferID id = 0;
    do id++;
    while (api.draw_indirect_buffers.find(id) != api.draw_indirect_buffers.end() or id == no_indirect_buffer);
    
    api.draw_indirect_buffers[id] = IndirectCommandBuffer{};
    IndirectCommandBuffer &icmd = api.draw_indirect_buffers[id];
    
    ui32 size = MAX_INDIRECT_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
    icmd.buffer = Common::allocateBuffer(size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    icmd.current_offset = 0;
    icmd.free_positions = {};
    
    return id;
}

void Graphics::addIndirectDrawCommand(DrawDescription &description) {
    GeometryBufferData &geometry = api.geometry_buffer_data.at(description.geometry);
    bool is_instanced = api.instanced_buffer_data.count(description.instance);
    
    //: Draw indirect command
    std::vector<VkDrawIndexedIndirectCommand> cmd(1);
    cmd.at(0).instanceCount = is_instanced ? api.instanced_buffer_data.at(description.instance).instance_count : 1;
    cmd.at(0).firstInstance = 0; //instance.instance_count * i++;
    cmd.at(0).firstIndex = 0;
    cmd.at(0).indexCount = geometry.index_size;
    
    //: Find available offset
    for (auto &[id, icmd] : api.draw_indirect_buffers) {
        if (icmd.free_positions.size() > 0) {
            description.indirect_buffer = id;
            description.indirect_offset = icmd.free_positions.at(0);
            icmd.free_positions.erase(icmd.free_positions.begin());
            break;
        }
        if (icmd.current_offset + 1 == MAX_INDIRECT_COMMANDS)
            continue;
        description.indirect_buffer = id;
        description.indirect_offset = icmd.current_offset++;
    }
    if (description.indirect_buffer == no_indirect_buffer) {
        description.indirect_buffer = registerIndirectCommandBuffer();
        description.indirect_offset = api.draw_indirect_buffers.at(description.indirect_buffer).current_offset++;
    }
    
    //: Update command buffer
    VK::updateGPUBuffer(api.draw_indirect_buffers.at(description.indirect_buffer).buffer, cmd, description.indirect_offset);
}

VkCommandBuffer VK::beginSingleUseCommandBuffer(VkDevice device, VkCommandPool pool) {
    //---Begin command buffer (single use)---
    //      Helper function which provides boilerplate for creating a single use command buffer
    //      It has the VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT flag, and it is allocated in the "transfer" command pool
    //      This with endSingleUseCommandBuffer() "sandwitches" the actual command code
    
    //: Allocation info
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = pool;
    allocate_info.commandBufferCount = 1;
    
    //: Allocate command buffer
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);
    
    //: Begin recording the buffer
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void VK::endSingleUseCommandBuffer(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool pool, VkQueue queue) {
    //---End command buffer (single use)---
    //      Helper function that complements beginSingleUseCommandBuffer() and finishes recording on a single use command buffer
    //      This part submits and frees the command buffer
    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    vkFreeCommandBuffers(device, pool, 1, &command_buffer);
}

#ifdef DEBUG

VkQueryPool VK::createQueryPool(VkQueryType type) {
    //---Query pool---
    //      This will allocate queries that we can perform, for example, to measure the execution time for a command buffer
    VkQueryPool query_pool;
    
    VkQueryPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    create_info.queryType = type;
    create_info.flags = 0;
    
    //: Timestamp
    if (type == VK_QUERY_TYPE_TIMESTAMP) {
        create_info.queryCount = Config::frames_in_flight * 2;
        //: One query before and after each draw call for each shader, and one for the entire gpu
    }
    
    //: Pipeline statistics
    if (type == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        create_info.pipelineStatistics =
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
        create_info.queryCount = Config::frames_in_flight * 6;
    }

    if (vkCreateQueryPool(api.device, &create_info, nullptr, &query_pool) != VK_SUCCESS)
        log::error("Failed to create a query pool");
    
    deletion_queue_program.push_back([query_pool](){ vkDestroyQueryPool(api.device, query_pool, nullptr); });
    
    return query_pool;
}

std::vector<ui64> VK::getQueryResults(VkQueryPool query, ui32 offset, ui32 count) {
    std::vector<ui64> results(count);
    
    vkGetQueryPoolResults(api.device, query, offset, count, count * sizeof(ui64), results.data(), sizeof(ui64), VK_QUERY_RESULT_64_BIT);
    
    return results;
}

#endif

//----------------------------------------



//Attachments
//----------------------------------------

VkSampleCountFlagBits VK::getAttachmentSamples(const Vulkan &vk, const AttachmentData &attachment) {
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    
    if (attachment.type & ATTACHMENT_MSAA) {
        auto get_samples = [](ui8 s){
            if (s == 1) return VK_SAMPLE_COUNT_2_BIT;
            if (s == 2) return VK_SAMPLE_COUNT_4_BIT;
            if (s == 3) return VK_SAMPLE_COUNT_8_BIT;
            if (s == 4) return VK_SAMPLE_COUNT_16_BIT;
            if (s == 5) return VK_SAMPLE_COUNT_32_BIT;
            if (s == 6) return VK_SAMPLE_COUNT_64_BIT;
            return VK_SAMPLE_COUNT_1_BIT;
        };
        
        samples = get_samples(Config::multisampling);
            
        auto available_counts = vk.physical_device_properties.limits.framebufferColorSampleCounts &
                                vk.physical_device_properties.limits.framebufferDepthSampleCounts;
        
        if (not (available_counts & samples)) {
            for (ui8 i = Config::multisampling - 1; i >= 0; i--) {
                VkSampleCountFlagBits fallback_samples = get_samples(i);
                if (available_counts & fallback_samples) {
                    samples = fallback_samples;
                    Config::multisampling = i;
                    log::warn("Using an unsupported number of MSSA samples, reducing to the nearest value, %d", i);
                    break;
                }
            }
        }
    }
    return samples;
}

VkAttachmentDescription VK::createAttachmentDescription(const AttachmentData &attachment, VkSampleCountFlagBits samples) {
    VkAttachmentDescription description{};
    
    description.format = attachment.format;
    description.samples = samples;
    description.loadOp = attachment.load_op;
    description.storeOp = attachment.store_op;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = attachment.initial_layout;
    description.finalLayout = attachment.final_layout;
    
    return description;
}

AttachmentID Graphics::registerAttachment(AttachmentType type, Vec2<ui16> size) {
    static AttachmentID id = 0;
    while (api.attachments.find(id) != api.attachments.end())
        id++;
    
    api.attachments[id] = AttachmentData{};
    AttachmentData &attachment = api.attachments.at(id);
    
    attachment.type = type;
    attachment.size = size;
    
    attachment.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    attachment.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    if (type & ATTACHMENT_COLOR) {
        attachment.usage = VkImageUsageFlagBits(attachment.usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        attachment.aspect = VkImageAspectFlagBits(attachment.aspect | VK_IMAGE_ASPECT_COLOR_BIT);
        attachment.format = api.swapchain.format;
        attachment.final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    
    if (type & ATTACHMENT_DEPTH) {
        attachment.usage = VkImageUsageFlagBits(attachment.usage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        attachment.aspect = VkImageAspectFlagBits(attachment.aspect | VK_IMAGE_ASPECT_DEPTH_BIT);
        attachment.format = VK::getDepthFormat(api.physical_device);
        attachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    
    if (type & ATTACHMENT_INPUT) {
        attachment.usage = VkImageUsageFlagBits(attachment.usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        attachment.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    }
    
    if (type & ATTACHMENT_EXTERNAL) {
        attachment.usage = VkImageUsageFlagBits(attachment.usage | VK_IMAGE_USAGE_SAMPLED_BIT);
        attachment.final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    }
    
    if (type & ATTACHMENT_SWAPCHAIN) {
        attachment.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.store_op = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    
    if (hasMultisampling(id)) { //: Multisampling
        if (type & ATTACHMENT_COLOR)
            attachment.final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (type & ATTACHMENT_DEPTH)
            attachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    
    if (hasMultisampling(id - 1)) //: Resolve
        attachment.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    
    VkSampleCountFlagBits samples = VK::getAttachmentSamples(api, attachment);
    
    //: Image and image view
    if (not (type & ATTACHMENT_SWAPCHAIN)) {
        auto [i_, a_] = VK::createImage(api.device, api.allocator, VMA_MEMORY_USAGE_GPU_ONLY, size, samples, 1,
                                        attachment.format, attachment.initial_layout, attachment.usage);
        attachment.image = i_;
        attachment.allocation = a_;
        
        attachment.image_view = VK::createImageView(api.device, attachment.image, attachment.aspect, attachment.format);
        VK::deletion_queue_swapchain.push_back([attachment](){ vkDestroyImageView(api.device, attachment.image_view, nullptr); });
    }
    
    //: Description
    attachment.description = VK::createAttachmentDescription(attachment, samples);
    
    return id;
}

void Graphics::recreateAttachments() {
    for (auto &[idx, attachment] : api.attachments) {
        if (attachment.type & ATTACHMENT_WINDOW) {
            attachment.size = to_vec(api.swapchain.extent);
        }
        
        VkSampleCountFlagBits samples = VK::getAttachmentSamples(api, attachment);
        
        if (not (attachment.type & ATTACHMENT_SWAPCHAIN)) {
            auto [i_, a_] = VK::createImage(api.device, api.allocator, VMA_MEMORY_USAGE_GPU_ONLY, attachment.size, samples, 1,
                                            attachment.format, attachment.initial_layout, attachment.usage);
            attachment.image = i_;
            attachment.allocation = a_;
            
            attachment.image_view = VK::createImageView(api.device, attachment.image, attachment.aspect, attachment.format);
            
            attachment.description = VK::createAttachmentDescription(attachment, samples);
            
            AttachmentData &attachment_ref = api.attachments.at(idx);
            VK::deletion_queue_swapchain.push_back([attachment_ref](){ vkDestroyImageView(api.device, attachment_ref.image_view, nullptr); });
        }
    }
}

VkFramebuffer VK::createFramebuffer(VkDevice device, VkRenderPass render_pass, std::vector<VkImageView> attachments, VkExtent2D extent) {
    //---Framebuffer---
    VkFramebuffer framebuffer;
    
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    //: Associated render pass
    create_info.renderPass = render_pass;
    
    //: The image view it will be rendering to
    create_info.attachmentCount = (ui32)attachments.size();
    create_info.pAttachments = attachments.data();
    
    //: Size and layers
    create_info.width = extent.width;
    create_info.height = extent.height;
    create_info.layers = 1;
    
    //: Create the framebuffer
    if (vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer) != VK_SUCCESS)
        log::error("Failed to create a vulkan framebuffer");
    
    return framebuffer;
}

std::vector<VkFramebuffer> VK::createFramebuffers(VkDevice device, RenderPassID r_id, VkExtent2D extent, const SwapchainData &swapchain) {
    //---Framebuffers---
    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(swapchain.size);
    
    auto attachments = getAtoB_v(r_id, Map::renderpass_attachment, api.attachments);
    RenderPassData &render = api.render_passes.at(r_id);
    
    for (int i = 0; i < swapchain.size; i++) {
        std::vector<VkImageView> fb_attachments{};
        for (auto &[id, data] : attachments) {
            fb_attachments.push_back(data.type & ATTACHMENT_SWAPCHAIN ? swapchain.image_views[i] : data.image_view);
            if (hasMultisampling(id, false))
                fb_attachments.push_back(api.attachments.at(id + 1).image_view);
        }
        framebuffers[i] = VK::createFramebuffer(device, render.render_pass, fb_attachments, extent);
    }
    
    deletion_queue_swapchain.push_back([framebuffers](){
        for (VkFramebuffer fb : framebuffers)
            vkDestroyFramebuffer(api.device, fb, nullptr);
    });
    log::graphics("Created vulkan framebuffers");
    return framebuffers;
}

//----------------------------------------



//Render Pass
//----------------------------------------

SubpassID Graphics::registerSubpass(std::vector<AttachmentID> attachment_list, std::vector<AttachmentID> external_attachment_list) {
    static SubpassID id = 0;
    while (api.subpasses.find(id) != api.subpasses.end())
        id++;
    
    log::graphics("Registering subpass %d:", id);
    api.subpasses[id] = SubpassData{};
    SubpassData &subpass = api.subpasses[id];
    
    subpass.external_attachments = external_attachment_list;
    
    for (auto &a_id : attachment_list)
        Map::subpass_attachment.add(id, a_id);
    
    for (auto &a_id : attachment_list) {
        const AttachmentData &data = api.attachments.at(a_id);
        bool first_in_chain = true;
        
        if (data.type & ATTACHMENT_INPUT) {
            //: Check if it is in one of the previous subpasses
            for (int i = id - 1; i >= 0; i--) {
                auto previous_attachments = getAtoB<AttachmentID>(i, Map::subpass_attachment);
                if (std::count(previous_attachments.begin(), previous_attachments.end(), a_id)) {
                    first_in_chain = false;
                    subpass.attachment_descriptions[a_id] = ATTACHMENT_INPUT;
                    subpass.previous_subpass_dependencies[a_id] = i;
                    log::graphics(" - Input attachment: %d (Depends on subpass %d)", a_id, i);
                    break;
                }
            }
        }
        
        if (first_in_chain) { //: First subpass that uses this attachment
            if (data.type & ATTACHMENT_COLOR) {
                subpass.attachment_descriptions[a_id] = ATTACHMENT_COLOR;
                log::graphics(" - Color attachment: %d", a_id);
            }
            if (data.type & ATTACHMENT_DEPTH) {
                subpass.attachment_descriptions[a_id] = ATTACHMENT_DEPTH;
                log::graphics(" - Depth attachment: %d", a_id);
            }
        }
    }
    
    return id;
}

RenderPassData VK::createRenderPass(const Vulkan &vk, RenderPassID r_id) {
    //---Render pass---
    //      All rendering happens inside of a render pass
    //      It can have multiple subpasses and attachments
    //      It will render to a framebuffer
    RenderPassData render{};
    VkRenderPassHelperData render_pass_helper{};
    
    //---Subpass list---
    auto subpasses = getAtoB_v(r_id, Map::renderpass_subpass, api.subpasses);
    std::map<SubpassID, ui8> relative_subpasses{}; ui8 i = 0;
    for (auto &[s_id, data] : subpasses)
        relative_subpasses[s_id] = i++;
    
    //---Attachment list---
    auto attachments = getAtoB_v(r_id, Map::renderpass_attachment, api.attachments);
    std::map<AttachmentID, ui32> relative_att_map{}; ui32 j = 0;
    for (auto &[a_id, data] : attachments) {
        relative_att_map[a_id] = j++;
        render_pass_helper.attachments.push_back(data.description);
        if (hasMultisampling(a_id, false)) {
            relative_att_map[a_id + 1] = j++;
            render_pass_helper.attachments.push_back(api.attachments.at(a_id + 1).description);
        }
    }
    
    //---Relative thematic attachment lists---
    std::map<SubpassID, std::vector<VkAttachmentReference>> relative_color_attachments{};
    std::map<SubpassID, VkAttachmentReference> relative_depth_attachment{};
    std::map<SubpassID, std::vector<VkAttachmentReference>> relative_input_attachments{};
    std::map<SubpassID, std::vector<VkAttachmentReference>> relative_resolve_attachments{};
    //---Lists for dependencies---
    std::map<SubpassID, std::vector<AttachmentID>> input_attachments{};
    std::vector<SubpassID> swapchain_attachments{};
    
    //---Subpass descriptions---
    render.attachment_extent = VkExtent2D{0, 0};
    for (auto &[s_id, data] : subpasses) {
        ui8 relative_subpass = relative_subpasses.at(s_id);
        
        //: Description
        render_pass_helper.subpasses.push_back(VkSubpassDescription{});
        VkSubpassDescription &description = render_pass_helper.subpasses.back();
        description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        
        //: Attachments
        int depth_count = 0; bool has_swapchain_attachment = false;
        for (auto &[a_id, a_type] : data.attachment_descriptions) {
            auto &a_data = api.attachments.at(a_id);
            
            VkExtent2D attachment_extent = to_extent(a_data.size);
            if (render.attachment_extent.width == 0 or render.attachment_extent.height == 0)
                render.attachment_extent = attachment_extent;
            else if (not (render.attachment_extent == attachment_extent))
                log::error("All attachments being redenred to in a render pass must be of the same size");
            
            if (a_type == ATTACHMENT_COLOR) {
                relative_color_attachments[s_id].push_back(VkAttachmentReference{relative_att_map.at(a_id), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                if (a_data.type & ATTACHMENT_SWAPCHAIN)
                    has_swapchain_attachment = true;
            }
            if (a_type == ATTACHMENT_DEPTH) {
                if (depth_count++ > 0)
                    log::error("Using more than one depth attachment in a subpass is not allowed");
                relative_depth_attachment[s_id] = VkAttachmentReference{relative_att_map.at(a_id), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            }
            if (a_type == ATTACHMENT_INPUT) {
                AttachmentID id = (hasMultisampling(a_id)) ? a_id + 1 : a_id;
                relative_input_attachments[s_id].push_back(VkAttachmentReference{relative_att_map.at(id), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
                input_attachments[s_id].push_back(id);
            }
            else if (hasMultisampling(a_id) and a_type == ATTACHMENT_COLOR) {
                VkAttachmentReference reference{};
                reference.attachment = relative_att_map.at(a_id + 1);
                reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                relative_resolve_attachments[s_id].push_back(reference);
            }
        }
        
        //: Description fields
        if (relative_color_attachments.count(s_id)) {
            description.colorAttachmentCount = (ui32)relative_color_attachments.at(s_id).size();
            description.pColorAttachments = relative_color_attachments.at(s_id).data();
        }
        
        if (depth_count > 0) {
            description.pDepthStencilAttachment = &relative_depth_attachment.at(s_id);
        }
        
        if (relative_input_attachments.count(s_id)) {
            description.inputAttachmentCount = (ui32)relative_input_attachments.at(s_id).size();
            description.pInputAttachments = relative_input_attachments.at(s_id).data();
        }
        
        if (relative_resolve_attachments.count(s_id)) {
            description.pResolveAttachments = relative_resolve_attachments.at(s_id).data();
        }
        
        //---Dependencies---
        //: Input (Ensure that the previous subpass has finished writting to the attachment this uses)
        if (input_attachments.count(s_id)) {
            for (const auto &a_id : input_attachments.at(s_id)) {
                VkSubpassDependency dependency;
                
                dependency.srcSubpass = relative_subpasses.at(data.previous_subpass_dependencies.at(hasMultisampling(a_id - 1) ? a_id - 1 : a_id));
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                
                dependency.dstSubpass = (ui32)relative_subpass;
                dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                
                dependency.dependencyFlags = 0;
                
                render_pass_helper.dependencies.push_back(dependency);
                
                log::graphics("Subpass dependency between %d and %d", dependency.srcSubpass, dependency.dstSubpass);
            }
        }
        //: Swapchain (Ensure that swapchain images are transitioned before they are written to)
        if (has_swapchain_attachment) {
            ui32 src = VK_SUBPASS_EXTERNAL;
            if (swapchain_attachments.size() > 0)
                src = swapchain_attachments.back();
            swapchain_attachments.push_back(relative_subpass);
            
            VkSubpassDependency dependency;
            
            dependency.srcSubpass = src;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            
            dependency.dstSubpass = (ui32)relative_subpass;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            
            dependency.dependencyFlags = 0;
            
            render_pass_helper.dependencies.push_back(dependency);
            log::graphics("Subpass dependency swapchain between %s and %d", (src == VK_SUBPASS_EXTERNAL ? "VK_SUBPASS_EXTERNAL" : std::to_string(src)).c_str(), dependency.dstSubpass);
        }
    }
    
    //---Create render pass---
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    
    create_info.attachmentCount = (ui32)render_pass_helper.attachments.size();
    create_info.pAttachments = render_pass_helper.attachments.data();
    
    create_info.subpassCount = (ui32)render_pass_helper.subpasses.size();
    create_info.pSubpasses = render_pass_helper.subpasses.data();
    
    create_info.dependencyCount = (ui32)render_pass_helper.dependencies.size();
    create_info.pDependencies = render_pass_helper.dependencies.data();
    
    if (vkCreateRenderPass(api.device, &create_info, nullptr, &render.render_pass) != VK_SUCCESS)
        log::error("Error creating a vulkan render pass");
    
    VK::deletion_queue_swapchain.push_back([render, vk](){ vkDestroyRenderPass(api.device, render.render_pass, nullptr); });
    log::graphics("Created a vulkan render pass with %d subpasses and %d attachments",
                  render_pass_helper.subpasses.size(), render_pass_helper.attachments.size());
    
    return render;
}

RenderPassID Graphics::registerRenderPass(std::vector<SubpassID> subpasses) {
    static RenderPassID id = 0;
    while (api.render_passes.find(id) != api.render_passes.end())
        id++;
    
    log::graphics("Registering render pass %d:", id);
    str s_list = std::accumulate(subpasses.begin(), subpasses.end(), str{""}, [](str s, SubpassID subpass){ return s + " " + std::to_string(subpass); });
    log::graphics("It contains subpasses %s", s_list.c_str());
    for (auto &s : subpasses)
        Map::renderpass_subpass.add(id, s);
    
    //---Create render pass---
    api.render_passes[id] = VK::createRenderPass(api, id);
    
    //---Create framebuffers---
    api.render_passes[id].framebuffers = VK::createFramebuffers(api.device, id, api.render_passes[id].attachment_extent, api.swapchain);
    
    return id;
}

void VK::recreateRenderPasses(Vulkan &vk) {
    for (auto &[id, render] : api.render_passes) {
        render = VK::createRenderPass(vk, id);
        render.framebuffers = VK::createFramebuffers(api.device, id, render.attachment_extent, vk.swapchain);
    }
}

//----------------------------------------



//Shaders
//----------------------------------------

IShaderModule Common::createInternalShaderModule(const std::vector<ui32> &code, ShaderStage stage) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size() * sizeof(ui32);
    create_info.pCode = code.data();
    
    VkShaderModule shader_module;
    if (vkCreateShaderModule(api.device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
        log::error("Error creating a Vulkan Shader Module");
        
    VK::deletion_queue_program.push_back([shader_module](){ vkDestroyShaderModule(api.device, shader_module, nullptr); });
    return shader_module;
}

std::vector<VkPipelineShaderStageCreateInfo> VK::getShaderStageInfo(const ShaderPass &pass) {
    std::vector<VkPipelineShaderStageCreateInfo> info;
    for (auto &stage : pass.stages) {
        VkPipelineShaderStageCreateInfo create_info{};
        
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = (ShaderStageT)stage.stage;
        create_info.module = stage.module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        
        info.push_back(create_info);
    }
    
    return info;
}

//----------------------------------------



//Sync objects
//----------------------------------------

SyncData VK::createSyncObjects() {
    //---Sync objects---
    //      Used to control the flow of operations when executing commands
    //      - Fence: GPU->CPU, we can wait from the CPU until a fence has finished on a GPU operation
    //      - Semaphore: GPU->GPU, can be signal or wait
    //          - Signal: Operation locks semaphore when executing and unlocks after it is finished
    //          - Wait: Wait until semaphore is unlocked to execute the command
    SyncData sync;
    sync.current_frame = 0;
    
    //---Frames in flight---
    //      Defined in r_vulkan_api.h, indicates how many frames can be processed concurrently
    
    //---Semaphores--- (GPU->GPU)
    //: Image available, locks when vkAcquireNextImageKHR() is getting a new image, then submits command buffer
    //: Render finished, locks while the command buffer is in execution, then finishes frame
    
    //: Default semaphore creation info
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    //---Fences--- (GPU->CPU)
    //: Frame in flight, waits until the frame is not in flight and can be writter again
    //: Images in flight, we need to track for each swapchain image if a frame in flight is currently using it, has size of swapchain
    sync.fences_images_in_flight.resize(api.swapchain.size, VK_NULL_HANDLE);
    
    //: Default fence creation info, signaled bit means they start like they have already finished once
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    //---Create semaphores and fences---
    for (int i = 0; i < Config::frames_in_flight; i++) {
        if (vkCreateSemaphore(api.device, &semaphore_info, nullptr, &sync.semaphores_image_available.at(i)) != VK_SUCCESS)
            log::error("Failed to create a vulkan semaphore (image available)");
        
        if (vkCreateSemaphore(api.device, &semaphore_info, nullptr, &sync.semaphores_render_finished.at(i)) != VK_SUCCESS)
            log::error("Failed to create a vulkan semaphore (render finished)");
        
        if (vkCreateFence(api.device, &fence_info, nullptr, &sync.fences_in_flight.at(i)) != VK_SUCCESS)
            log::error("Failed to create a vulkan fence (frame in flight)");
    }
    
    deletion_queue_program.push_back([sync](){
        for (int i = 0; i < Config::frames_in_flight; i++) {
            vkDestroySemaphore(api.device, sync.semaphores_image_available.at(i), nullptr);
            vkDestroySemaphore(api.device, sync.semaphores_render_finished.at(i), nullptr);
            vkDestroyFence(api.device, sync.fences_in_flight.at(i), nullptr);
        }
    });
    log::graphics("Created all vulkan sync objects");
    log::graphics("");
    
    return sync;
}

//----------------------------------------



//Pipeline
//----------------------------------------

VkPipelineInputAssemblyStateCreateInfo VK::Pipeline::getInputAssembly(VkPrimitiveTopology topology) {
    //---Input assembly info---
    //      Here we specify the way the vertices will be processed, for example a triangle list (Each 3 vertices will form a triangle)
    //      Other possible configurations make it possible to draw points or lines
    //      If primitiveRestartEnable is set to true, it is possible to break strips using a special index, but we won't use it yet
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    
    input_assembly.topology = topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    
    return input_assembly;
}

VkPipelineRasterizationStateCreateInfo VK::Pipeline::getRasterizer(VkPolygonMode polygon_mode, VkCullModeFlagBits cull, float line_width) {
    //---Rasterizer info---
    //      Configuration for the rasterizer stage
    //      While it is not directly programmable like the vertex or fragment stage, we can set some variables to modify how it works
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    
    //: Polygon modes
    //      - Fill, fills the triangle area with fragments
    //      - Line, draws the edges of the triangle as lines
    //      - Point, only draws the vertices of the triangle as points
    //      (Line and point require enabling GPU features)
    rasterizer.polygonMode = polygon_mode;
    
    //: Line width
    //      Describes the thickness of lines in term of fragments, it requires enabling the wideLines GPU feature
    rasterizer.lineWidth = line_width;
    
    //: Culling (disabled)
    //      We will be culling the back face to save in performance
    //      To calculate the front face, if we are not sending normals, the vertices will be calculated in counter clockwise order
    //      If nothing shows to the screen, one of the most probable causes is the winding order of the vertices to be reversed
    rasterizer.cullMode = cull;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VK::Pipeline::getMultisampling() {
    //---Multisampling info---
    //      It is one of the ways to perform anti-aliasing when multiple polygons rasterize to the same pixel
    //      We will get back to this and enable it
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    
    multisampling.sampleShadingEnable = VK_FALSE;
    
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    
    return multisampling;
}

VkPipelineColorBlendAttachmentState VK::Pipeline::getColorBlend() {
    //---Color blending attachment---
    //      Specifies how colors should be blended together (disabled by default)
    VkPipelineColorBlendAttachmentState color_blend{};
    
    color_blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend.blendEnable = VK_FALSE;

    //: One possible configuration is alpha blending:
    //      final_color.rgb = new_color.a * new_color.rgb + (1 - new_color.a) * old_color.rgb;
    //      final_color.a = new_color.a;
    
    /*color_blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend.colorBlendOp = VK_BLEND_OP_ADD;
    
    color_blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend.alphaBlendOp = VK_BLEND_OP_ADD;*/
    
    return color_blend;
}

VkPipelineDepthStencilStateCreateInfo VK::Pipeline::getDepthStencil(bool depth_test, bool depth_write, VkCompareOp compare_op) {
    //---Depth info---
    //      WORK IN PROGRESS, get back to this
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    depth_stencil.depthTestEnable = depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = depth_write ? VK_TRUE : VK_FALSE;
    
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    
    depth_stencil.depthCompareOp = depth_test ? compare_op : VK_COMPARE_OP_ALWAYS;
    
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    
    return depth_stencil;
}

VkVertexInputBindingDescription VK::Pipeline::getVertexBindingDescription(ui32 size, ui32 binding, VkVertexInputRate rate) {
    VkVertexInputBindingDescription v;
    
    v.stride = size;
    v.binding = binding;
    v.inputRate = rate;
    
    return v;
}

PipelineCreateData VK::Pipeline::getCreateData(ConfigData config, ShaderID shader) {
    PipelineCreateData data{};
    
    //: Vertex info
    data.vertex_input_binding_descriptions = {};
    data.vertex_input_attribute_descriptions = {};
    
    ui32 binding = 0;
    ui32 previous_location = 0;
    
    static const std::map<VertexFormat, VkFormat> format_map = {
        {VERTEX_FORMAT_R_F, VK_FORMAT_R32_SFLOAT},
        {VERTEX_FORMAT_RG_F, VK_FORMAT_R32G32_SFLOAT},
        {VERTEX_FORMAT_RGB_F, VK_FORMAT_R32G32B32_SFLOAT},
        {VERTEX_FORMAT_RGBA_F, VK_FORMAT_R32G32B32A32_SFLOAT},
    };
    
    for (auto v : config.vertex_descriptions) {
        for_<VertexType>([&](auto i) {
            using V = std::variant_alternative_t<i.value, VertexType>;
            
            str vertex_name = str(type_name<V>());
            if (vertex_name.rfind("Vertex", 0) != 0) log::error("All vertex types need to start with 'Vertex', this is %s", vertex_name.c_str());
            vertex_name = lower(vertex_name.substr(6));
            
            if (vertex_name == lower(v.first)) {
                //: Binding descriptions
                data.vertex_input_binding_descriptions.push_back(getVertexBindingDescription(sizeof(V), binding, (VkVertexInputRate)v.second));
                
                //: Attribute descriptions
                auto attributes = getAttributeDescriptions<V>(binding, previous_location);
                previous_location += (ui32)attributes.size();
                for (auto &a : attributes) {
                    data.vertex_input_attribute_descriptions.push_back(
                        VkVertexInputAttributeDescription{a.location, a.binding, format_map.at(a.format), a.offset});
                }
            }
        });
        binding++;
        if (data.vertex_input_binding_descriptions.size() != binding)
            log::error("The vertex '%s' didn't match any of the VertexType names", v.first.c_str());
    }
    
    //: Input assembly (primitive topology)
    data.input_assembly = getInputAssembly(config.topology);

    //: Rasterization state (polygon mode, face culling)
    data.rasterizer = getRasterizer(config.polygon_mode, config.cull, config.line_width);
    
    //: Multisampling
    data.multisampling = getMultisampling();
    auto subpass = getBtoA_v(shader, Map::subpass_shader, api.subpasses);
    auto attachments = getAtoB_v(subpass.first, Map::subpass_attachment, api.attachments);
    std::optional<VkSampleCountFlagBits> samples;
    for (auto &[id, a] : attachments) {
        if (std::count(subpass.second.external_attachments.begin(), subpass.second.external_attachments.end(), id)) //: Skip external attachments
            continue;
        if (subpass.second.attachment_descriptions.at(id) == ATTACHMENT_INPUT) //: Skip input attachments when they are used as input (already resolved)
            continue;
        if (not samples.has_value())
            samples = a.description.samples;
        if (samples.value() != a.description.samples)
            log::error("All attachments that are not external must have the same number of samples %d, attachment: %d - %d",
                       samples.value(), id, a.description.samples);
    }
    if (samples.has_value())
        data.multisampling.rasterizationSamples = samples.value();
    //TODO: Recreate pipelines when multisampling changes
    
    //: Color blend
    data.color_blend_attachment = getColorBlend();
    
    //: Depth stencil
    data.depth_stencil = getDepthStencil(config.depth_test, config.depth_write, config.compare_op);
    
    //: Shader and layout
    data.shader = shader;
    data.layout = Shader::getShader(shader).pipeline_layout;
    
    return data;
}

VkPipelineLayout VK::createPipelineLayout(const std::vector<VkDescriptorSetLayout> &set_layouts,
                                          const std::vector<VkPushConstantRange> &push_constants) {
    //---Pipeline layout---
    //      Holds the information of the descriptor set layouts that we created earlier
    //      This allows to reference uniforms or images at draw time and change them without recreating the pipeline
    VkPipelineLayout pipeline_layout;
    
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    create_info.setLayoutCount = (ui32)set_layouts.size();
    create_info.pSetLayouts = set_layouts.data();
    
    create_info.pushConstantRangeCount = (ui32)push_constants.size();
    create_info.pPushConstantRanges = push_constants.data();
    
    if (vkCreatePipelineLayout(api.device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        log::error("Error creating a vulkan pipeline layout");
    
    deletion_queue_program.push_back([pipeline_layout](){ vkDestroyPipelineLayout(api.device, pipeline_layout, nullptr); });
    log::graphics("Created a vulkan pipeline layout");
    return pipeline_layout;
}

VkPipeline VK::buildGraphicsPipeline(const PipelineCreateData &data) {
    //---Pipeline---
    //      The graphics pipeline is a series of stages that convert vertex and other data into a visible image that can be shown to the screen
    //      Input assembler -> Vertex shader -> Tesselation -> Geometry shader -> Rasterization -> Fragment shader -> Color blending -> Frame
    //      Here we put together all the previous helper functions and structs
    //      It holds shader stages, all the creation info, a layout for description sets, render passes...
    
    //: Vertex input info
    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    vertex_input.vertexBindingDescriptionCount = (ui32)data.vertex_input_binding_descriptions.size();
    vertex_input.pVertexBindingDescriptions = data.vertex_input_binding_descriptions.data();
    
    vertex_input.vertexAttributeDescriptionCount = (ui32)data.vertex_input_attribute_descriptions.size();
    vertex_input.pVertexAttributeDescriptions = data.vertex_input_attribute_descriptions.data();
    
    //: Viewport
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;
    
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &data.viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &data.scissor;
    
    //: Color blending
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &data.color_blend_attachment;
    
    //: Dynamic state
    std::vector<VkDynamicState> states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS};
    
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pDynamicStates = states.data();
    dynamic_state.dynamicStateCount = (ui32)states.size();
    
    //: Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = getShaderStageInfo(Shader::getShader(data.shader));
    
    //: Render pass
    auto renderpass = getBtoA_v(data.shader, Map::renderpass_shader, api.render_passes);
    auto subpass_list = getAtoB<SubpassID>(renderpass.first, Map::renderpass_subpass);
    auto subpass = getBtoA_v(data.shader, Map::subpass_shader, api.subpasses);
    
    auto it = std::find(subpass_list.begin(), subpass_list.end(), subpass.first);
    if (it == subpass_list.end())
        log::error("The subpass %d is not part of the render pass %d", subpass.first, renderpass.first);
    ui32 relative_subpass = (ui32)(it - subpass_list.begin());
    
    //: Add all pipeline create info
    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    
    create_info.pVertexInputState = &vertex_input;
    create_info.pViewportState = &viewport_state;
    create_info.pColorBlendState = &color_blending;
    create_info.pDynamicState = &dynamic_state;
    
    create_info.stageCount = (ui32)shader_stages.size();
    create_info.pStages = shader_stages.data();
    
    create_info.pInputAssemblyState = &data.input_assembly;
    create_info.pRasterizationState = &data.rasterizer;
    create_info.pMultisampleState = &data.multisampling;
    create_info.pDepthStencilState = &data.depth_stencil;
    
    create_info.layout = data.layout;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    create_info.renderPass = renderpass.second.render_pass;
    create_info.subpass = relative_subpass;
    
    //: Build pipeline
    VkPipeline pipeline;
    
    if (vkCreateGraphicsPipelines(api.device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        log::error("Error while creating a vulkan graphics pipeline");
    log::graphics("Created a vulkan graphics pipeline");
    
    //: Cleanup
    deletion_queue_program.push_back([pipeline](){ vkDestroyPipeline(api.device, pipeline, nullptr); });
    return pipeline;
}

IPipeline Common::createGraphicsPipeline(ShaderID shader, std::vector<std::pair<str, VertexInputRate>> vertices) {
    log::graphics("Pipeline %s", shader.c_str());
    
    Common::linkDescriptorResources(shader);
    VK::linkPipelineDescriptors(shader);
    
    VK::Pipeline::ConfigData config{};
    config.vertex_descriptions = vertices;
    
    PipelineCreateData create_info = VK::Pipeline::getCreateData(config, shader);
    return VK::buildGraphicsPipeline(create_info);
}

//----------------------------------------



//Buffers
//----------------------------------------

BufferData Common::allocateBuffer(ui32 size, BufferUsage usage, BufferMemory memory, void* data, bool delete_with_program) {
    //---Buffers---
    //      These are regions of memory that store arbitrary data that the CPU, GPU or both can read
    //      We are using Vulkan Memory Allocator for allocating their memory in a bigger pool
    BufferData buffer;
    
    //---Create buffer---
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.usage = (BufferUsageT)usage;

    if (memory == BUFFER_MEMORY_GPU_ONLY)
        create_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    
    VmaAllocationCreateInfo allocate_info{};
    allocate_info.usage = (BufferMemoryT)memory;
    
    if (vmaCreateBuffer(api.allocator, &create_info, &allocate_info, &buffer.buffer, &buffer.allocation, nullptr) != VK_SUCCESS)
        log::error("Failed to create a vulkan buffer");
    
    if (delete_with_program)
        VK::deletion_queue_program.push_back([buffer](){ vmaDestroyBuffer(api.allocator, buffer.buffer, buffer.allocation); });
    
    //---No data provided, just allocation---
    if (data == nullptr)
        return buffer;
    
    //---Data provided, CPU buffer---
    if (memory != BUFFER_MEMORY_GPU_ONLY) {
        //: Copy data
        void* b;
        vmaMapMemory(api.allocator, buffer.allocation, &b);
        memcpy(b, data, (size_t)size);
        vmaUnmapMemory(api.allocator, buffer.allocation);
    }
    
    //---Data provided, GPU buffer---
    else {
        //: Staging buffer
        //      We want to make the vertex buffer accessible to the GPU in the most efficient way, so we use a staging buffer
        //      This is created in CPU only memory, in which we can easily map the vertex data
        BufferData staging_buffer = allocateBuffer(size, BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_CPU_ONLY, nullptr, false);
        
        //: Map data to staging buffer
        void* b;
        vmaMapMemory(api.allocator, staging_buffer.allocation, &b);
        memcpy(b, data, (size_t)size);
        vmaUnmapMemory(api.allocator, staging_buffer.allocation);
        
        //: Copy from staging to buffer
        //      Since we can't access the buffer memory from the CPU, we will use vkCmdCopyBuffer(), which will execute on a queue
        //      and move data between the staging and buffer
        copyBuffer(staging_buffer, buffer, size);
        
        //: Delete staging buffer
        vmaDestroyBuffer(api.allocator, staging_buffer.buffer, staging_buffer.allocation);
    }
    
    return buffer;
}

void Common::copyBuffer(BufferData &src, BufferData &dst, ui32 size, ui32 offset) {
    //---Copy buffer---
    //      Simple function that creates a command buffer to copy the memory from one buffer to another
    //      Very helpful when using staging buffers to move information from CPU to GPU only memory (can be done in reverse)
    VkCommandBuffer command_buffer = VK::beginSingleUseCommandBuffer(api.device, api.cmd.command_pools.at("transfer"));
    
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = offset;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src.buffer, dst.buffer, 1, &copy_region);
    
    VK::endSingleUseCommandBuffer(api.device, command_buffer, api.cmd.command_pools.at("transfer"), api.cmd.queues.transfer);
}

void VK::updateBufferFromCompute(const BufferData &buffer, ui32 buffer_size, ShaderID shader) {
    //TODO: ENABLE COMPUTE SHADERS
    log::warn("Enable compute shaders");
    /*const PipelineData &pipeline = api.compute_pipelines.at(shader);
    
    //: Update pipeline descriptor sets to add the buffer and pipeline uniforms. For now this only works with a single buffer.
    std::vector<VkBuffer> uniforms{};
    for (auto &b : pipeline.uniform_buffers)
        uniforms.push_back(b.at(0).buffer);
    
    WriteDescriptors descriptors{};
    ui32 count = 0;
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER>(descriptors, shader, count, uniforms);
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_STORAGE_BUFFER>(descriptors, shader, count, std::vector<VkBuffer>{buffer.buffer});
    vkUpdateDescriptorSets(api.device, count, descriptors.write.data(), 0, nullptr);
    
    //: Begin one time command buffer
    VkCommandBuffer cmd = VK::beginSingleUseCommandBuffer(api.device, api.cmd.command_pools.at("compute"));
    
    //: Bind pipeline and descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline_layout, 0, 1, &compute_shaders.at(shader).descriptors.at(0).descriptors.at(0), 0, nullptr);
    
    //: Calculate group count and dispatch compute calls
    ui32 group_count = (buffer_size / pipeline.group_size[0]) + 1;
    vkCmdDispatch(cmd, group_count, 1, 1);
    
    //: End and submit command buffer
    VK::endSingleUseCommandBuffer(api.device, cmd, api.cmd.command_pools.at("compute"), api.cmd.queues.compute);*/
}

//----------------------------------------



//Descriptors
//----------------------------------------

VkDescriptorSetLayout VK::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
    //---Create the descriptor layout itself---
    VkDescriptorSetLayout layout;
    
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = (ui32)bindings.size();
    create_info.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(api.device, &create_info, nullptr, &layout) != VK_SUCCESS)
        log::error("Error creating a descriptor set layout, check shader reflection");
    
    deletion_queue_program.push_back([layout](){ vkDestroyDescriptorSetLayout(api.device, layout, nullptr); });
    return layout;
}

IDescriptorPool Common::createDescriptorPool(const std::vector<IDescriptorPoolSize> &pool_sizes) {
    //---Descriptor pool---
    //      A descriptor pool will house descriptor sets of various kinds
    //      There are two types of usage for a descriptor pool:
    //      - Allocate sets once at the start of the program, and then use them each time
    //        This is what we are doing here, so we can know the exact pool size and destroy the pool at the end
    //      - Allocate sets per frame, this can be implemented in the future
    //        It can be cheap using VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT and resetting the entire pool per frame
    //        We would have a list of descriptor pools with big sizes for each type of descriptor, and if an allocation fails,
    //        just create another pool and add it to the list. At the end of the frame all of them get deleted.
    VkDescriptorPool descriptor_pool;
    
    //: Create info
    //      This uses the descriptor pool size from the list of sizes specified in r_shaders.h
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = (ui32)pool_sizes.size();
    create_info.pPoolSizes = pool_sizes.data();
    create_info.maxSets = Shader::descriptor_pool_max_sets;
    
    //: Create descriptor pool
    if (vkCreateDescriptorPool(api.device, &create_info, nullptr, &descriptor_pool) != VK_SUCCESS)
        log::error("Failed to create a vulkan descriptor pool");
    log::graphics("Created a vulkan descriptor pool");
    
    //: Cleanup and return
    VK::deletion_queue_program.push_back([descriptor_pool](){ vkDestroyDescriptorPool(api.device, descriptor_pool, nullptr); });
    return descriptor_pool;
}

std::vector<VkDescriptorSet> Common::allocateDescriptorSets(IDescriptorLayout layout, IDescriptorPool *pool) {
    //: Allocate information
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = *pool;
    allocate_info.descriptorSetCount = Config::frames_in_flight;
    std::vector<VkDescriptorSetLayout> layouts(Config::frames_in_flight, layout);
    allocate_info.pSetLayouts = layouts.data();
    
    //: Create the descriptor set
    std::vector<VkDescriptorSet> descriptors(Config::frames_in_flight);
    if (vkAllocateDescriptorSets(api.device, &allocate_info, descriptors.data()) == VK_SUCCESS)
        return descriptors;
    return {};
}

void VK::updateDescriptorSets(ShaderID shader, std::vector<std::array<VkBuffer, Config::frames_in_flight>> uniform_buffers,
                              std::vector<VkBuffer> storage_buffers, std::vector<VkImageView> image_views, std::vector<VkImageView> input_attachments) {
    WriteDescriptors descriptors{};
    ui32 count = 0;
    
    //: Uniform buffers
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER>(descriptors, shader, count, uniform_buffers);
    //: Storage buffers
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_STORAGE_BUFFER>(descriptors, shader, count, storage_buffers);
    //: Images
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER>(descriptors, shader, count, image_views);
    //: Input attachments
    VK::prepareWriteDescriptor<VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT>(descriptors, shader, count, input_attachments);
    
    vkUpdateDescriptorSets(api.device, count, descriptors.write.data(), 0, nullptr);
}

void Graphics::updateDrawDescriptorSets(const DrawDescription& draw) {
    //: Uniforms
    std::vector<std::array<VkBuffer, Config::frames_in_flight>> uniform_buffers{};
    for (auto &v : api.draw_uniform_data.at(draw.uniform).uniform_buffers) {
        uniform_buffers.resize(uniform_buffers.size() + 1);
        for (int i = 0; i < Config::frames_in_flight; i++)
            uniform_buffers.back().at(i) = v.at(i).buffer;
    }
    
    //: Images
    std::vector<VkImageView> image_views{};
    if (draw.texture != no_texture) image_views.push_back(api.texture_data.at(draw.texture).image_view);
    
    //: Update
    VK::updateDescriptorSets(draw.shader, uniform_buffers, {}, image_views, {});
}

void VK::linkPipelineDescriptors(ShaderID shader) {
    std::vector<VkImageView> image_views{};
    std::vector<VkImageView> input_views{};
    
    auto subpass = getBtoA_v(shader, Map::subpass_shader, api.subpasses);
    
    for (auto &a : subpass.second.external_attachments)
        image_views.push_back(api.attachments.at(hasMultisampling(a) ? a + 1 : a).image_view);
        
    for (auto &[a, type] : subpass.second.attachment_descriptions)
        if (type == ATTACHMENT_INPUT)
            input_views.push_back(api.attachments.at(hasMultisampling(a) ? a + 1 : a).image_view);
    
    VK::updateDescriptorSets(shader, {}, {}, image_views, input_views);
}

void Common::linkDescriptorResources(ShaderID shader) {
    auto &descriptors = Shader::getShader(shader).descriptors;
    
    std::vector<std::array<VkBuffer, Config::frames_in_flight>> uniform_buffers{};
    
    for (auto &d : descriptors) {
        for (auto &r : d.resources) {
            if (r.type == DESCRIPTOR_UNIFORM) {
                std::array<VkBuffer, Config::frames_in_flight> u{};
                for (int i = 0; i < Config::frames_in_flight; i++)
                    u.at(i) = descriptor_resources.uniform_buffers.at(r.id + i).buffer;
                uniform_buffers.push_back(u);
            }
        }
    }
    
    VK::updateDescriptorSets(shader, uniform_buffers, {}, {}, {});
}

//----------------------------------------



//Images
//----------------------------------------

TextureID Graphics::registerTexture(Vec2<ui16> size, Channels ch, ui8* pixels) {
    //---Create texture---
    static TextureID id = 0;
    do id++;
    while (api.texture_data.find(id) != api.texture_data.end() and id == no_texture);
    
    //: Format
    VkFormat format = [ch](){
        switch(ch) {
            case TEXTURE_CHANNELS_G:
                return VK_FORMAT_R8_UNORM;
            case TEXTURE_CHANNELS_GA:
                return VK_FORMAT_R8G8_UNORM;
            case TEXTURE_CHANNELS_RGB:
                return VK_FORMAT_R8G8B8_UNORM;
            case TEXTURE_CHANNELS_RGBA:
                return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }();
    
    //: Texture
    api.texture_data[id] = VK::createTexture(api.device, api.allocator, api.physical_device,
                                             VkImageUsageFlagBits(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                             VK_IMAGE_ASPECT_COLOR_BIT, size, format, ch);
    
    //: Staging buffer
    ui32 buffer_size = size.x * size.y * (int)ch * sizeof(ui8);
    BufferData staging_buffer = Common::allocateBuffer(buffer_size, BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_CPU_ONLY, nullptr, false);
    
    //: Map pixels to staging buffer
    void* data;
    vmaMapMemory(api.allocator, staging_buffer.allocation, &data);
    memcpy(data, pixels, (size_t) buffer_size);
    vmaUnmapMemory(api.allocator, staging_buffer.allocation);
    
    //: Copy from staging buffer to image and transition to read only layout
    VK::transitionImageLayoutCmd(api.device, api.cmd, api.texture_data[id], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VK::copyBufferToImage(api.device, api.cmd, staging_buffer, api.texture_data[id]);
    VK::transitionImageLayoutCmd(api.device, api.cmd, api.texture_data[id], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    //: Delete staging buffer
    vmaDestroyBuffer(api.allocator, staging_buffer.buffer, staging_buffer.allocation);
    
    return id;
}

TextureData VK::createTexture(VkDevice device, VmaAllocator allocator, VkPhysicalDevice physical_device,
                              VkImageUsageFlagBits usage, VkImageAspectFlagBits aspect, Vec2<ui16> size, VkFormat format, Channels ch) {
    //---Texture---
    TextureData data{};
    
    //: Format
    data.format = format;
    data.ch = ch;
    
    //: Size
    data.w = size.x;
    data.h = size.y;
    
    //: Layout (changed later)
    data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    //: Image
    auto [i_, a_] = VK::createImage(device, allocator, VMA_MEMORY_USAGE_GPU_ONLY, size, VK_SAMPLE_COUNT_1_BIT, 1, data.format, data.layout, usage);
    data.image = i_;
    data.allocation = a_;
    
    //: Image view
    data.image_view = VK::createImageView(device, data.image, aspect, data.format);
    deletion_queue_program.push_back([data](){ vkDestroyImageView(api.device, data.image_view, nullptr); });
    
    return data;
}

std::pair<VkImage, VmaAllocation> VK::createImage(VkDevice device, VmaAllocator allocator, VmaMemoryUsage memory, Vec2<ui16> size,
                                                  VkSampleCountFlagBits samples, ui32 mip_levels, VkFormat format,
                                                  VkImageLayout layout, VkImageUsageFlags usage) {
    VkImage image;
    VmaAllocation allocation;
    
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = (ui32)size.x;
    create_info.extent.height = (ui32)size.y;
    create_info.extent.depth = 1;
    create_info.mipLevels = mip_levels;
    create_info.arrayLayers = 1;
    
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL; //This way the image can't be access directly, but it is better for performance
    create_info.format = format;
    create_info.initialLayout = layout;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    create_info.samples = samples;
    create_info.flags = 0;
    
    VmaAllocationCreateInfo allocate_info{};
    allocate_info.usage = memory;
    
    if (vmaCreateImage(allocator, &create_info, &allocate_info, &image, &allocation, nullptr) != VK_SUCCESS)
        log::error("Failed to create a vulkan image");
    
    deletion_queue_program.push_back([allocator, image, allocation](){
        vmaDestroyImage(allocator, image, allocation);
    });
    
    return std::pair<VkImage, VmaAllocation>{image, allocation};
}

void VK::transitionImageLayout(VkDevice device, VkCommandBuffer cmd, VkImage image, VkFormat format,
                               VkImageLayout old_layout, VkImageLayout new_layout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.subresourceRange.aspectMask = [&]() -> VkImageAspectFlagBits {
        if (new_layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            return VK_IMAGE_ASPECT_COLOR_BIT;
        
        if (hasDepthStencilComponent(format))
            return (VkImageAspectFlagBits)(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
        
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }();
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;
    
    barrier.srcAccessMask = [&](){
        switch (old_layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                return (VkAccessFlagBits)0;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                src_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                return VK_ACCESS_SHADER_READ_BIT;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                src_stage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                return VK_ACCESS_SHADER_READ_BIT;
            default:
                log::warn("Not a valid src access mask in image layout transition");
                return (VkAccessFlagBits)0;
        }
    }();
    
    barrier.dstAccessMask = [&](){
        switch (new_layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                return VK_ACCESS_TRANSFER_READ_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                return VK_ACCESS_SHADER_READ_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                return (VkAccessFlagBits)(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
            default:
                log::warn("Not a valid dst access mask in image layout transition");
                return (VkAccessFlagBits)0;
        }
    }();
    
    vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VK::transitionImageLayoutCmd(VkDevice device, const CommandData &cmd, TextureData &tex, VkImageLayout new_layout) {
    VkCommandBuffer command_buffer = beginSingleUseCommandBuffer(device, cmd.command_pools.at("temp"));

    VK::transitionImageLayout(device, command_buffer, tex.image, tex.format, tex.layout, new_layout);
    
    endSingleUseCommandBuffer(device, command_buffer, cmd.command_pools.at("temp"), cmd.queues.graphics);
    
    tex.layout = new_layout;
}

void VK::copyBufferToImage(VkDevice device, const CommandData &cmd, BufferData &buffer, TextureData &tex) {
    VkCommandBuffer command_buffer = beginSingleUseCommandBuffer(device, cmd.command_pools.at("transfer"));
    
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;

    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;

    copy_region.imageOffset = {0, 0, 0};
    copy_region.imageExtent = {(ui32)tex.w, (ui32)tex.h, 1};
    
    vkCmdCopyBufferToImage(command_buffer, buffer.buffer, tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    
    endSingleUseCommandBuffer(device, command_buffer, cmd.command_pools.at("transfer"), cmd.queues.transfer);
}

VkImageView VK::createImageView(VkDevice device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format) {
    VkImageViewCreateInfo create_info{};
    
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    
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
        log::error("Error creating a vulkan image view");
    
    return image_view;
}

VkSampler VK::createSampler(VkDevice device) {
    VkSampler sampler;
    
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    create_info.magFilter = VK_FILTER_NEAREST; //This is for pixel art, change to linear for interpolation
    create_info.minFilter = VK_FILTER_NEAREST;
    
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; //The texture doesn't repeat if sampled out of range
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    
    create_info.anisotropyEnable = VK_FALSE;
    create_info.maxAnisotropy = 1.0f;
    
    //Enable for anisotropy, as well as device features in createDevice()
    /*VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vk.physical_device, &properties);
    create_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;*/
    
    create_info.unnormalizedCoordinates = VK_FALSE;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0f;
    create_info.maxLod = 0.0f;
    
    if (vkCreateSampler(device, &create_info, nullptr, &sampler) != VK_SUCCESS)
        log::error("Error creating a Vulkan image sampler");
    
    deletion_queue_program.push_back([sampler](){ vkDestroySampler(api.device, sampler, nullptr); });
    return sampler;
}

//----------------------------------------



//Depth
//----------------------------------------
VkFormat VK::getDepthFormat(VkPhysicalDevice physical_device) {
    //---Choose depth format---
    //      Get the most appropiate format for the depth images of the framebuffer
    return chooseSupportedFormat(physical_device, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                 VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool VK::hasDepthStencilComponent(VkFormat format) {
    //---Checks if depth texture has stencil component---
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
//----------------------------------------



//Render
//----------------------------------------

ui8 VK::startRender() {
    ui32 image_index;
    
    std::vector<VkFence> fences = { api.sync.fences_in_flight.at(api.sync.current_frame) };
    vkWaitForFences(api.device, (ui32)fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
    
    VkResult result = vkAcquireNextImageKHR(api.device, api.swapchain.swapchain, UINT64_MAX,
                                            api.sync.semaphores_image_available.at(api.sync.current_frame), VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        VK::recreateSwapchain();
        return UINT8_MAX;
    }
    
    if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR)
        log::error("Failed to acquire swapchain image");
    
    if (api.sync.fences_images_in_flight.at(image_index) != VK_NULL_HANDLE)
        vkWaitForFences(api.device, 1, &api.sync.fences_images_in_flight.at(image_index), VK_TRUE, UINT64_MAX);
    
    api.sync.fences_images_in_flight.at(image_index) = api.sync.fences_in_flight.at(api.sync.current_frame);
    
    return (ui8)image_index;
}

void VK::renderFrame() {
    VkSemaphore wait_semaphores[]{ api.sync.semaphores_image_available.at(api.sync.current_frame) };
    VkPipelineStageFlags wait_stages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[]{ api.sync.semaphores_render_finished.at(api.sync.current_frame) };
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    
    std::vector<VkCommandBuffer> command_buffers = { api.cmd.command_buffers.at("draw").at(api.sync.current_frame) };
    IF_GUI(command_buffers.push_back( api.cmd.command_buffers.at("gui").at(api.sync.current_frame) ));
    submit_info.commandBufferCount = (ui32)command_buffers.size();
    submit_info.pCommandBuffers = command_buffers.data();
    
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    vkResetFences(api.device, 1, &api.sync.fences_in_flight.at(api.sync.current_frame));
    
    if (vkQueueSubmit(api.cmd.queues.graphics, 1, &submit_info, api.sync.fences_in_flight.at(api.sync.current_frame)) != VK_SUCCESS)
        log::error("Failed to submit Draw Command Buffer");
}

//----------------------------------------


//Test
//----------------------------------------

void Graphics::render() {
    static std::vector<DrawDescription*> previous_draw_descriptions{};
    
    //: Get the current image
    api.cmd.current_image = VK::startRender();
    
    //: Remove indirect buffers that are no longer in use
    if (Config::draw_indirect) {
        if (previous_draw_descriptions != api.draw_descriptions) {
            for (auto description : previous_draw_descriptions) {
                if (std::find(api.draw_descriptions.begin(), api.draw_descriptions.end(), description) == api.draw_descriptions.end())
                    removeIndirectDrawCommand(*description);
            }
        }
    }
    
    //: Update uniform buffers
    //      Keep an eye on how performant is this, another solution might be necessary
    for (auto &f : VK::update_buffer_queue) f(api.sync.current_frame);
    VK::update_buffer_queue.clear();
    
    //: Record command buffers
    VK::recordRenderCommandBuffer();
    IF_GUI(VK::Gui::recordGuiCommandBuffer());
    
    //: Render the frame
    VK::renderFrame();
    
    //: Clear draw queue
    previous_draw_descriptions = api.draw_descriptions;
    api.draw_queue_instanced.clear();
    api.draw_descriptions.clear();
}

void Graphics::present() {
    VkSemaphore signal_semaphores[]{ api.sync.semaphores_render_finished.at(api.sync.current_frame) };
    
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[]{ api.swapchain.swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = (ui32*)&api.cmd.current_image;
    present_info.pResults = nullptr;
    
    VkResult result = vkQueuePresentKHR(api.cmd.queues.present, &present_info);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
        VK::recreateSwapchain();
    else if (result != VK_SUCCESS)
        log::error("Failed to present Swapchain Image");
    
    api.sync.current_frame = (api.sync.current_frame + 1) % Config::frames_in_flight;
}

//----------------------------------------



//Resize
//----------------------------------------

void Graphics::resize() {
    VK::recreateSwapchain();
    
    WindowTransform transform = {glm::mat4(1.f), glm::mat4(1.f)};
    if (camera.projection & PROJECTION_SCALED)
        transform = Window::getScaledTransform(Config::resolution);
    
    //: Update scaled projection
    for (auto &[id, shader] : shaders.list.at(SHADER_POST)) {
        if (id.rfind("window", 0) == 0) {
            auto &u = shader.descriptors.at(0).resources.at(0); //Hardcoded to be in set 0, descriptor 0
            for (int i = 0; i < u.count; i++)
                updateUniformBuffer(descriptor_resources.uniform_buffers.at(u.id + i), transform);
        }
    }
}

//----------------------------------------



//Cleanup
//----------------------------------------

void Graphics::clean() {
    vkDeviceWaitIdle(api.device);
    
    //: Delete swapchain objects
    for (auto it = VK::deletion_queue_swapchain.rbegin(); it != VK::deletion_queue_swapchain.rend(); ++it)
        (*it)();
    VK::deletion_queue_swapchain.clear();
    
    //: Delete program resources
    for (auto it = VK::deletion_queue_program.rbegin(); it != VK::deletion_queue_program.rend(); ++it)
        (*it)();
    VK::deletion_queue_program.clear();
    
    log::graphics("Cleaned up Vulkan");
}

//----------------------------------------



//Debug
//----------------------------------------

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanReportFunc(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData)

{
    printf("[VULKAN]: %s\n", msg);
    return VK_FALSE;
}

PFN_vkCreateDebugReportCallbackEXT SDL2_vkCreateDebugReportCallbackEXT = nullptr;

VkDebugReportCallbackEXT VK::createDebug(VkInstance &instance) {
    SDL2_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)SDL_Vulkan_GetVkGetInstanceProcAddr();

    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_callback_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debug_callback_create_info.pfnCallback = vulkanReportFunc;

    VkDebugReportCallbackEXT debug_callback;
    
    SDL2_vkCreateDebugReportCallbackEXT(instance, &debug_callback_create_info, 0, &debug_callback);
    
    return debug_callback;
}

//----------------------------------------



//Gui
//----------------------------------------

#ifndef DISABLE_GUI
void VK::Gui::init(Vulkan &vk) {
    //: Init SDL implementation
    if (not ImGui_ImplSDL2_InitForVulkan(window.window))
        log::error("Error initializing ImGui for SDL");
    
    //: Create descriptor pool
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    Gui::descriptor_pool = Common::createDescriptorPool(pool_sizes);
    
    //: Render pass
    AttachmentID attachment_swapchain_gui = registerAttachment(ATTACHMENT_COLOR_SWAPCHAIN, window.size);
    SubpassID subpass_gui = registerSubpass({attachment_swapchain_gui});
    vk.gui_render_pass = registerRenderPass({subpass_gui});
    
    //: Init Vulkan implementation
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk.instance;
    init_info.PhysicalDevice = vk.physical_device;
    init_info.Device = api.device;
    init_info.QueueFamily = vk.cmd.queue_indices.graphics.value();
    init_info.Queue = vk.cmd.queues.graphics;
    init_info.DescriptorPool = Gui::descriptor_pool;
    init_info.MinImageCount = vk.swapchain.min_image_count;
    init_info.ImageCount = vk.swapchain.size;
    init_info.Allocator = nullptr;
    if (not ImGui_ImplVulkan_Init(&init_info, api.render_passes.at(vk.gui_render_pass).render_pass))
        log::error("Error initializing ImGui for Vulkan");
    
    //: Command buffers
    vk.cmd.command_buffers["gui"] = VK::allocateDrawCommandBuffers();
    
    //: Cleanup
    deletion_queue_program.push_back([](){
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    });
}

void VK::Gui::transferFonts(const Vulkan &vk) {
    //: Transfer fonts
    VkCommandBuffer command_buffer = Graphics::VK::beginSingleUseCommandBuffer(api.device, vk.cmd.command_pools.at("transfer"));
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    Graphics::VK::endSingleUseCommandBuffer(api.device, command_buffer, vk.cmd.command_pools.at("transfer"), vk.cmd.queues.transfer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VK::Gui::recordGuiCommandBuffer() {
    const VkCommandBuffer &cmd = api.cmd.command_buffers.at("gui").at(api.sync.current_frame);
    
    //: Begin buffer
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        log::error("Failed to begin recording on a vulkan gui command buffer");
    
    //: Clear values
    std::vector<VkClearValue> clear_values { {0.f, 0.f, 0.f, 1.0f} };
    
    //: Begin render pass
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = api.render_passes.at(api.gui_render_pass).render_pass;
    render_pass_info.framebuffer = api.render_passes.at(api.gui_render_pass).framebuffers.at(api.cmd.current_image);
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = api.render_passes.at(api.gui_render_pass).attachment_extent;
    render_pass_info.clearValueCount = (ui32)clear_values.size();
    render_pass_info.pClearValues = clear_values.data();
    
    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    //: ImGui draw
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    
    //: End render pass
    vkCmdEndRenderPass(cmd);
    
    //: End command buffer
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        log::error("Failed to end recording on a vulkan command buffer");
}
#endif

//----------------------------------------

#endif
