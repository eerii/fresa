//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "r_vulkan_api.h"
#include "r_api.h"

#include "r_shader.h"
#include "r_vertex.h"
#include "r_textures.h"

#include "ftime.h"
#include "config.h"

#include <set>
#include <numeric>
#include <glm/gtc/matrix_transform.hpp>

using namespace Verse;
using namespace Graphics;

namespace {
    const std::vector<VertexData> vertices = {
        {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}}, //Light
        {{1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}}, //Teal
        {{1.f, 1.f, -1.f}, {1.000f, 0.815f, 0.019f}}, //Yellow
        {{-1.f, 1.f, -1.f}, {0.988f, 0.521f, 0.113f}}, //Orange
        {{-1.f, -1.f, 1.f}, {0.925f, 0.254f, 0.345f}}, //Red
        {{1.f, -1.f, 1.f}, {0.925f, 0.235f, 0.647f}}, //Pink
        {{1.f, 1.f, 1.f}, {0.658f, 0.180f, 0.898f}}, //Purple
        {{-1.f, 1.f, 1.f}, {0.258f, 0.376f, 0.941f}}, //Blue
    };

    const std::vector<ui16> indices = {
        0, 3, 1, 3, 2, 1,
        1, 2, 5, 2, 6, 5,
        4, 7, 0, 7, 3, 0,
        3, 7, 2, 7, 6, 2,
        4, 0, 5, 0, 1, 5,
        5, 6, 4, 6, 7, 4,
    };

    const std::vector<const char*> validation_layers{
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> required_device_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    std::vector<std::function<void()>> deletion_queue_program;
    std::vector<std::function<void()>> deletion_queue_swapchain_extra;
    std::vector<std::function<void()>> deletion_queue_swapchain;
    std::vector<std::function<void()>> deletion_queue_frame;

    std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{};
}



//Initialization
//----------------------------------------

void API::configure() {
    
}

Vulkan API::create(WindowData &win) {
    Vulkan vk;
    
    //---Instance---
    vk.instance = VK::createInstance(win);
    vk.debug_callback = VK::createDebug(vk.instance);
    
    //---Surface---
    vk.surface = VK::createSurface(vk.instance, win);
    
    //---Physical device---
    vk.physical_device = VK::selectPhysicalDevice(vk.instance, vk.surface);
    vk.physical_device_features = VK::getPhysicalDeviceFeatures(vk.physical_device);
    
    //---Queues and logical device---
    vk.cmd.queue_indices = VK::getQueueFamilies(vk.surface, vk.physical_device);
    vk.device = VK::createDevice(vk.physical_device, vk.physical_device_features, vk.cmd.queue_indices);
    vk.cmd.queues = VK::getQueues(vk.device, vk.cmd.queue_indices);
    
    //---Allocator---
    vk.allocator = VK::createMemoryAllocator(vk.device, vk.physical_device, vk.instance);
    
    //---Swapchain---
    vk.swapchain = VK::createSwapchain(vk.device, vk.physical_device, vk.surface, vk.cmd.queue_indices, win);
    
    //---Command pools---
    vk.cmd.command_pools = VK::createCommandPools(vk.device, vk.cmd.queue_indices,
        {"draw", "temp", "transfer"},
        {{"transfer", vk.cmd.queue_indices.transfer.value()}},
        {{"draw", VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT},
         {"temp", VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
         {"transfer", VK_COMMAND_POOL_CREATE_TRANSIENT_BIT}});
    vk.cmd.command_buffers["draw"] = VK::allocateDrawCommandBuffers(vk.device, vk.swapchain.size, vk.cmd);
    
    //---Render pass---
    vk.swapchain.main_render_pass = VK::createRenderPass(vk.device, vk.swapchain.format);
    
    //---Framebuffers---
    vk.swapchain.framebuffers = VK::createFramebuffers(vk.device, vk.swapchain);
    
    //---Sync objects---
    vk.sync = VK::createSyncObjects(vk.device, vk.swapchain.size);
    
    //---Shader data---
    vk.shader = VK::createShaderData(vk.device, "res/shaders/test/test.vert.spv", "res/shaders/test/test.frag.spv");
    
    //---Descriptor set---
    vk.descriptors.layout = VK::createDescriptorSetLayout(vk.device, vk.shader.code, vk.swapchain.size);
    vk.descriptors.pool = VK::createDescriptorPool(vk.device);
    vk.descriptors.sets = VK::allocateDescriptorSets(vk.device, vk.descriptors.layout, vk.descriptors.pool, vk.swapchain.size);
    
    //---Pipeline---
    vk.pipeline_layout = VK::createPipelineLayout(vk.device, vk.descriptors.layout);
    vk.pipeline = VK::createGraphicsPipeline(vk.device, vk.pipeline_layout, vk.swapchain, vk.shader.stages);
    
    //---Create resources---
    API::prepareResources(vk);
    
    //---Record command buffers---
    for (int i = 0; i < vk.swapchain.size; i++)
        VK::recordDrawCommandBuffer(vk, i);
    
    log::graphics("");
    
    return vk;
}

void API::prepareResources(Vulkan &vk) {
    //---Resource phase---
    //      In this stage we will create the buffers and images that the program will use
    //      Some won't change for the duration of the program (vertex buffers, images...) and some are placeholders that we will update in
    //      a per frame basis, but it makes sense to allocate them now
    
    //---Immutable buffers---
    //      Allocated and filled up now
    vk.vertex_buffer = VK::createVertexBuffer(vk.device, vk.allocator, vk.cmd, vertices);
    vk.index_buffer = VK::createIndexBuffer(vk.device, vk.allocator, vk.cmd, indices);
    vk.index_buffer_size = (ui32)indices.size();
    
    //---Uniform buffers---
    //      Allocated now and updated every frame
    vk.uniform_buffers = VK::createUniformBuffers(vk.allocator, vk.swapchain.size);
    
    //---Images---
    //VK::createSampler(vk);
    //vk.test_image = Texture::load(vk, "res/graphics/texture.png", Texture::TEXTURE_CHANNELS_RGBA);
    
    //---Update descriptor sets---
    //      After creating all the resources, they need to be added to the descriptor sets
    VK::updateDescriptorSets(vk.device, vk.descriptors.sets, vk.swapchain.size, vk.uniform_buffers);
}

//----------------------------------------



//Device
//----------------------------------------

VkInstance VK::createInstance(const WindowData &win) {
    log::graphics("");
    
    //---Instance extensions---
    //      Add extra functionality to Vulkan
    ui32 extension_count;
    SDL_Vulkan_GetInstanceExtensions(win.window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(win.window, &extension_count, extension_names.data());
    log::graphics("Vulkan requested instance extensions: %d", extension_count);
    for (const auto &ext : extension_names)
        log::graphics(" - %s", ext);
    
    log::graphics("");
    
    //---Validation layers---
    //      Middleware for existing Vulkan functionality
    //      Primarily used for getting detailed error descriptions, in this case with VK_LAYER_KHRONOS_validation
    ui32 validation_layer_count;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
    log::graphics("Vulkan supported validation layers: %d", validation_layer_count);
    for (VkLayerProperties layer : available_validation_layers)
        log::graphics(" - %s", layer.layerName);
    
    log::graphics("");
    
    for (const auto &val : validation_layers) {
        bool layer_exists = false;
        for (const auto &layer : available_validation_layers) {
            if (str(val) == str(layer.layerName)) {
                layer_exists = true;
                break;
            }
        }
        if (not layer_exists)
            log::error("Attempted to use a validation layer but it is not supported (%s)", val);
    }
    
    //---App info---
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = Conf::name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(Conf::version[0], Conf::version[1], Conf::version[2]);
    app_info.pEngineName = "Fresa";
    app_info.engineVersion = VK_MAKE_VERSION(Conf::version[0], Conf::version[1], Conf::version[2]);
    app_info.apiVersion = VK_API_VERSION_1_1;
    
    //---Instance create info---
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = (int)validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
    instance_create_info.enabledExtensionCount = (int)extension_names.size();
    instance_create_info.ppEnabledExtensionNames = extension_names.data();
    
    //---Create instance---
    VkInstance instance;
    
    if (vkCreateInstance(&instance_create_info, nullptr, &instance)!= VK_SUCCESS)
        log::error("Fatal error creating a vulkan instance");
    
    deletion_queue_program.push_back([instance](){vkDestroyInstance(instance, nullptr);});
    return instance;
}

VkSurfaceKHR VK::createSurface(VkInstance instance, const WindowData &win) {
    VkSurfaceKHR surface;
    
    //---Surface---
    //      It is the abstraction of the window created by SDL to something Vulkan can draw onto
    if (not SDL_Vulkan_CreateSurface(win.window, instance, &surface))
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
    QueueIndices queue_indices = getQueueFamilies(surface, physical_device);
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
    VK::SwapchainSupportData swapchain_support = VK::getSwapchainSupport(surface, physical_device);
    if (swapchain_support.formats.empty() or swapchain_support.present_modes.empty())
        return 0;
    
    return score;
}

VkPhysicalDevice VK::selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
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
    
    
    //---Show the result of the process---
    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        log::graphics(((device == physical_device) ? " > %s" : " - %s"), device_properties.deviceName);
    }
    log::graphics("");
    
    if (physical_device == VK_NULL_HANDLE)
        log::error("No GPU passed the vulkan physical device requirements.");
    
    return physical_device;
}

VkPhysicalDeviceFeatures VK::getPhysicalDeviceFeatures(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);
    return device_features;
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

VK::QueueIndices VK::getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    //---Queues---
    //      Different execution ports of the GPU, command buffers are submitted here
    //      There are different spetialized queue families, such as present, graphics and compute
    
    
    //---Get all available queues---
    ui32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_list.data());
    
    
    //---Select desired queues---
    //      Using the QueueIndices struct, which has 3 std::optional indices, for:
    //          - Graphics: pipeline operations, including vertex/fragment shaders and drawing
    //          - Present: send framebuffers to the screen
    //          - Compute: for compute shaders
    //      Not all queues are needed, and in the future more queues can be created for multithread support
    //      Made so present and graphics queue can be the same
    QueueIndices queue_indices;
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
                          const QueueIndices &queue_indices) {
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
    //: (optional) Anisotropy - vk.physical_device_features.samplerAnisotropy = VK_TRUE;
    
    
    //---Create device---
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    //: Add the queues we selected before to be created
    device_create_info.queueCreateInfoCount = (int)queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = (int)required_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.pEnabledFeatures = &physical_device_features;
    
    device_create_info.enabledLayerCount = (int)validation_layers.size();
    device_create_info.ppEnabledLayerNames = validation_layers.data();
    
    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device)!= VK_SUCCESS)
        log::error("Error creating a vulkan logical device");
    
    deletion_queue_program.push_back([device](){vkDestroyDevice(device, nullptr);});
    return device;
}

VK::QueueData VK::getQueues(VkDevice device, const QueueIndices &queue_indices) {
    VK::QueueData queues;
    
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

VK::SwapchainSupportData VK::getSwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    SwapchainSupportData swapchain_support;
    
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
    //      We also select a nonlinear SRGB color space to correctly represent images
    //      If all fails, it will still select a format, but it might not be the perfect one
    for (const VkSurfaceFormatKHR &fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return fmt;
    }
    log::warn("A non ideal format has been selected for the swap surface, since BGRA SRGB is not supported. You might experience that the graphics present in unexpected colors. Please check the GPU support for ideal representation.");
    return formats[0];
}

VkPresentModeKHR VK::selectSwapPresentMode(const std::vector<VkPresentModeKHR> &modes) {
    //---Surface present mode---
    //      The way the buffers are swaped to the screen
    //      - Fifo: Vsync, when the queue is full the program waits
    //      - Mailbox: Triple buffering, the program replaces the last images of the queue, less latency but more power consumption
    //      Not all GPUs support mailbox (for example integrated Intel GPUs), so while it is preferred, Fifo can be used as well
    //      Maybe in the future offer the user the opportunity to choose the desired mode
    
    for (const VkPresentModeKHR &mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            log::graphics("Present mode: Mailbox");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    
    log::graphics("Present mode: Fifo");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VK::selectSwapExtent(VkSurfaceCapabilitiesKHR capabilities, const WindowData &win) {
    //---Surface extent---
    //      This is the drawable are on the screen
    //      If the current extent is UINT32_MAX, we should calculate the actual extent using WindowData
    //      and clamp it to the min and max supported extent by the GPU
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(win.window, &w, &h);
    
    VkExtent2D actual_extent{ static_cast<ui32>(w), static_cast<ui32>(h) };
    
    std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actual_extent;
}

VkSwapchainData VK::createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                    const QueueIndices &queue_indices, const WindowData &win) {
    //---Swapchain---
    //      List of images that will get drawn to the screen by the render pipeline
    //      Swapchain data:
    //      - VkFormat format: Image format of the swapchain, usually B8G8R8A8_SRGB
    //      - VkExtent2D extent: The size of the draw area
    //      - VkSwapchainKHR swapchain: Actual swapchain object
    //      - std::vector<VkImage> images: List of swapchain images
    //      - std::vector<VkImageView> image_views: List of their respective image views
    //      - TODO: Depth data
    
    VkSwapchainData swapchain;
    
    
    //---Format, present mode and extent---
    SwapchainSupportData support = getSwapchainSupport(surface, physical_device);
    VkSurfaceFormatKHR surface_format = selectSwapSurfaceFormat(support.formats);
    swapchain.format = surface_format.format;
    VkPresentModeKHR present_mode = selectSwapPresentMode(support.present_modes);
    swapchain.extent = selectSwapExtent(support.capabilities, win);
    
    
    //---Number of images---
    ui32 min_image_count = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 and min_image_count > support.capabilities.maxImageCount)
        min_image_count = support.capabilities.maxImageCount;
    
    
    //---Create swapchain---
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    
    //: Swapchain images
    create_info.minImageCount = min_image_count;
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
    deletion_queue_swapchain.push_back([swapchain, device](){
        for (VkImageView view : swapchain.image_views)
            vkDestroyImageView(device, view, nullptr);
        vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
    });
    
    log::graphics("Created a vulkan swapchain");
    return swapchain;
}

void VK::recreateSwapchain(Vulkan &vk, const WindowData &win) {
    //---Recreate swapchain---
    //      When the swapchain is no longer actual, for example on window resize, we need to recreate it
    
    //: Save previous size to avoid recreating non necessary things
    ui32 previous_size = vk.swapchain.size;
    
    //: Wait for draw operations to finish
    vkDeviceWaitIdle(vk.device);
    
    //: Clean previous swapchain
    for (auto it = deletion_queue_swapchain.rbegin(); it != deletion_queue_swapchain.rend(); ++it)
        (*it)();
    deletion_queue_swapchain.clear();
    
    //---Swapchain---
    vk.swapchain = createSwapchain(vk.device, vk.physical_device, vk.surface, vk.cmd.queue_indices, win);
    
    //: Clean objects that depend on the swapchain size
    if (previous_size != vk.swapchain.size) {
        for (auto it = deletion_queue_swapchain_extra.rbegin(); it != deletion_queue_swapchain_extra.rend(); ++it)
            (*it)();
        deletion_queue_swapchain_extra.clear();
    }
    
    //---Command buffer allocation---
    vk.cmd.command_buffers["draw"] = VK::allocateDrawCommandBuffers(vk.device, vk.swapchain.size, vk.cmd);
    
    //---Render pass---
    vk.swapchain.main_render_pass = VK::createRenderPass(vk.device, vk.swapchain.format);
    
    //---Framebuffers---
    vk.swapchain.framebuffers = VK::createFramebuffers(vk.device, vk.swapchain);
    
    //---Sync objects---
    if (previous_size != vk.swapchain.size)
        vk.sync = VK::createSyncObjects(vk.device, vk.swapchain.size);
    
    vk.pipeline = VK::createGraphicsPipeline(vk.device, vk.pipeline_layout, vk.swapchain, vk.shader.stages);
    
    if (previous_size != vk.swapchain.size) {
    //---Descriptor set---
        vk.descriptors.layout = VK::createDescriptorSetLayout(vk.device, vk.shader.code, vk.swapchain.size);
        vk.descriptors.pool = VK::createDescriptorPool(vk.device);
        vk.descriptors.sets = VK::allocateDescriptorSets(vk.device, vk.descriptors.layout, vk.descriptors.pool, vk.swapchain.size);
    
    //---Pipeline---
        vk.pipeline_layout = VK::createPipelineLayout(vk.device, vk.descriptors.layout);
        vk.pipeline = VK::createGraphicsPipeline(vk.device, vk.pipeline_layout, vk.swapchain, vk.shader.stages);
        
    //---Uniform buffers---
        vk.uniform_buffers = VK::createUniformBuffers(vk.allocator, vk.swapchain.size);
        
    //---Update descriptors---
        VK::updateDescriptorSets(vk.device, vk.descriptors.sets, vk.swapchain.size, vk.uniform_buffers);
    }
    
    //---Record command buffers---
    for (int i = 0; i < vk.swapchain.size; i++)
        VK::recordDrawCommandBuffer(vk, i);
}

VkFormat VK::getDepthFormat(Vulkan &vk) {
    //---Choose depth format---
    //      Get the most appropiate format for the depth images of the framebuffer
    return chooseSupportedFormat(vk.physical_device, {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                 VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void VK::createDepthResources(Vulkan &vk) {
    //---Depth resources---
    //PENDING
}

//----------------------------------------



//Commands
//----------------------------------------

std::map<str, VkCommandPool> VK::createCommandPools(VkDevice device, const QueueIndices &queue_indices, std::vector<str> keys,
                                                    std::map<str, ui32> queues, std::map<str, VkCommandPoolCreateFlagBits> flags) {
    //---Command pools---
    //      Command buffers can be allocated inside them
    //      We can have multiple command pools for different types of buffers, for example, "draw" and "transfer"
    //      Resetting the command pool is easier than individually resetting buffers
    
    std::map<str, VkCommandPool> command_pools;
    
    for (auto key: keys) {
        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        
        //: Queue index for this specific command pool
        create_info.queueFamilyIndex = [queues, queue_indices, key](){
            if (queues.count(key) > 0)
                return queues.at(key);
            return queue_indices.graphics.value();
        }();
        
        //: Flags, for example, a transient flag for temporary buffers
        create_info.flags = [flags, key](){
            if (flags.count(key) > 0)
                return flags.at(key);
            return (VkCommandPoolCreateFlagBits)0;
        }();
        
        //: Create the command pool
        if (vkCreateCommandPool(device, &create_info, nullptr, &command_pools[key]) != VK_SUCCESS)
            log::error("Failed to create a vulkan command pool (%s)", key.c_str());
    }
    
    deletion_queue_program.push_back([command_pools, device](){
        for (auto [key, pool] : command_pools)
            vkDestroyCommandPool(device, pool, nullptr);
    });
    log::graphics("Created all vulkan command pools");
    return command_pools;
}

std::vector<VkCommandBuffer> VK::allocateDrawCommandBuffers(VkDevice device, ui32 swapchain_size, const VkCommandData &cmd) {
    //---Command buffers---
    //      All vulkan commands must be executed inside a command buffer
    //      Here we create the command buffers we will use for drawing, and allocate them inside a command pool ("draw")
    //      We are creating one buffer per swapchain image
    
    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(swapchain_size);
    
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = cmd.command_pools.at("draw");
    allocate_info.commandBufferCount = (ui32)command_buffers.size();
    
    if (vkAllocateCommandBuffers(device, &allocate_info, command_buffers.data()) != VK_SUCCESS)
        log::error("Failed to allocate the vulkan main draw command buffers");
    
    log::graphics("Allocated the vulkan draw command buffers");
    
    deletion_queue_swapchain.push_back([cmd, command_buffers, device](){
        vkFreeCommandBuffers(device, cmd.command_pools.at("draw"), (ui32)command_buffers.size(), command_buffers.data());
    });
    return command_buffers;
}

void VK::beginDrawCommandBuffer(VkCommandBuffer cmd, VkPipeline pipeline, VkFramebuffer framebuffer,
                                VkRenderPass render_pass, VkExtent2D extent) {
    //---Begin draw command buffer---
    
    //:
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;
    
    //:
    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        log::error("Failed to begin recording on a vulkan command buffer");
    
    //:
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.01f, 0.01f, 0.05f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};
    
    //:
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = extent;
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values.data();
    
    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    //:
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void VK::endDrawCommandBuffer(VkCommandBuffer cmd, ui32 index_size) {
    //---End draw command buffer---
    
    //:
    vkCmdDrawIndexed(cmd, index_size, 1, 0, 0, 0);
    
    //:
    vkCmdEndRenderPass(cmd);
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        log::error("Failed to end recording on a vulkan command buffer");
}

void VK::recordDrawCommandBuffer(const Vulkan &vk, ui32 current) {
    //---Draw command buffer---
    
    const VkCommandBuffer &cmd = vk.cmd.command_buffers.at("draw")[current];
    
    //: TODO: WRITE THIS
    VK::beginDrawCommandBuffer(cmd, vk.pipeline, vk.swapchain.framebuffers[current],
                               vk.swapchain.main_render_pass, vk.swapchain.extent);
    
    //:
    VkBuffer vertex_buffers[]{ vk.vertex_buffer.buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
    
    //:
    vkCmdBindIndexBuffer(cmd, vk.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    
    //:
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.pipeline_layout, 0, 1, &vk.descriptors.sets[current], 0, nullptr);
    
    //:
    VK::endDrawCommandBuffer(cmd, vk.index_buffer_size);
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
    //      TODO: Make the free command work at the end of the frame, maybe with syncronization
    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    vkFreeCommandBuffers(device, pool, 1, &command_buffer);
}

//----------------------------------------



//Render Pass
//----------------------------------------

VkSubpassDescription VK::createRenderSubpass() {
    //---Render subpass---
    VkAttachmentReference color_attachment_ref{};
    
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    
    return subpass;
}

VkSubpassDependency VK::createRenderSubpassDependency() {
    //---Render subpass dependency---
    VkSubpassDependency dependency{};
    
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    return dependency;
}

VkAttachmentDescription VK::createRenderPassAttachment(VkFormat format) {
    //---Render pass attachment---
    VkAttachmentDescription attachment{};
    
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    return attachment;
}

VK::RenderPassCreateData VK::prepareRenderPass(VkFormat format) {
    //---Combine all data required to create a render pass---
    RenderPassCreateData data;
    
    data.subpasses = { VK::createRenderSubpass() };
    data.dependencies = { VK::createRenderSubpassDependency() };
    data.attachments = { VK::createRenderPassAttachment(format) };
    
    return data;
}

VkRenderPass VK::createRenderPass(VkDevice device, VkFormat format) {
    //---Render pass---
    //      All rendering happens inside of a render pass
    //      It can have multiple subpasses and attachments
    //      It will render to a framebuffer
    VkRenderPass render_pass;
    
    RenderPassCreateData render_pass_data = VK::prepareRenderPass(format);
    
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    
    create_info.attachmentCount = (ui32)render_pass_data.attachments.size();
    create_info.pAttachments = render_pass_data.attachments.data();
    
    create_info.subpassCount = (ui32)render_pass_data.subpasses.size();
    create_info.pSubpasses = render_pass_data.subpasses.data();
    
    create_info.dependencyCount = (ui32)render_pass_data.dependencies.size();
    create_info.pDependencies = render_pass_data.dependencies.data();
    
    if (vkCreateRenderPass(device, &create_info, nullptr, &render_pass) != VK_SUCCESS)
        log::error("Error creating a vulkan render pass");
    
    deletion_queue_swapchain.push_back([render_pass, device](){ vkDestroyRenderPass(device, render_pass, nullptr); });
    log::graphics("Created a vulkan render pass");
    return render_pass;
}

VkFramebuffer VK::createFramebuffer(VkDevice device, VkRenderPass render_pass, VkImageView image_view, VkExtent2D extent) {
    //---Framebuffer---
    VkFramebuffer framebuffer;
    
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    //: Associated render pass
    create_info.renderPass = render_pass;
    
    //: The image view it will be rendering to
    create_info.attachmentCount = 1;
    create_info.pAttachments = &image_view;
    
    //: Size and layers
    create_info.width = extent.width;
    create_info.height = extent.height;
    create_info.layers = 1;
    
    //: Create the framebuffer
    if (vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer) != VK_SUCCESS)
        log::error("Failed to create a vulkan framebuffer");
    
    return framebuffer;
}

std::vector<VkFramebuffer> VK::createFramebuffers(VkDevice device, const VkSwapchainData &swapchain) {
    //---Framebuffers---
    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(swapchain.size);
    
    for (int i = 0; i < swapchain.size; i++)
        framebuffers[i] = VK::createFramebuffer(device, swapchain.main_render_pass, swapchain.image_views[i], swapchain.extent);
    
    deletion_queue_swapchain.push_back([framebuffers, device](){
        for (VkFramebuffer fb : framebuffers)
            vkDestroyFramebuffer(device, fb, nullptr);
    });
    log::graphics("Created all vulkan framebuffers");
    return framebuffers;
}

//----------------------------------------



//Sync objects
//----------------------------------------

VkSyncData VK::createSyncObjects(VkDevice device, ui32 swapchain_size) {
    //---Sync objects---
    //      Used to control the flow of operations when executing commands
    //      - Fence: GPU->CPU, we can wait from the CPU until a fence has finished on a GPU operation
    //      - Semaphore: GPU->GPU, can be signal or wait
    //          - Signal: Operation locks semaphore when executing and unlocks after it is finished
    //          - Wait: Wait until semaphore is unlocked to execute the command
    VkSyncData sync;
    
    //---Frames in flight---
    //      Defined in r_vulkan_api.h, indicates how many frames can be processed concurrently
    
    
    //---Semaphores--- (GPU->GPU)
    //: Image available, locks when vkAcquireNextImageKHR() is getting a new image, then submits command buffer
    sync.semaphores_image_available.resize(MAX_FRAMES_IN_FLIGHT);
    //: Render finished, locks while the command buffer is in execution, then finishes frame
    sync.semaphores_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
    //: Default semaphore creation info
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    
    //---Fences--- (GPU->CPU)
    //: Frame in flight, waits until the frame is not in flight and can be writter again
    sync.fences_in_flight.resize(MAX_FRAMES_IN_FLIGHT);
    //: Images in flight, we need to track for each swapchain image if a frame in flight is currently using it, has size of swapchain
    sync.fences_images_in_flight.resize(swapchain_size, VK_NULL_HANDLE);
    //: Default fence creation info, signaled bit means they start like they have already finished once
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    
    //---Create semaphores and fences---
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &sync.semaphores_image_available[i]) != VK_SUCCESS)
            log::error("Failed to create a vulkan semaphore (image available)");
        
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &sync.semaphores_render_finished[i]) != VK_SUCCESS)
            log::error("Failed to create a vulkan semaphore (render finished)");
        
        if (vkCreateFence(device, &fence_info, nullptr, &sync.fences_in_flight[i]) != VK_SUCCESS)
            log::error("Failed to create a vulkan fence (frame in flight)");
    }
    
    
    deletion_queue_program.push_back([device, sync](){
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, sync.semaphores_image_available[i], nullptr);
            vkDestroySemaphore(device, sync.semaphores_render_finished[i], nullptr);
            vkDestroyFence(device, sync.fences_in_flight[i], nullptr);
        }
    });
    log::graphics("Created all vulkan sync objects");
    return sync;
}

//----------------------------------------



//Pipeline
//----------------------------------------

ShaderData VK::createShaderData(VkDevice device, str vert, str frag, str compute, str geometry) {
    //---Shader data---
    //      Creates a shader data object from a list of locations for the different stages
    //      First it saves the locations, then it reads the code, and then gets the stage create info
    ShaderData data;
    
    if (vert != "" and not data.locations.vert.has_value())
        data.locations.vert = vert;
    if (frag != "" and not data.locations.frag.has_value())
        data.locations.frag = frag;
    if (compute != "" and not data.locations.compute.has_value())
        data.locations.compute = compute;
    if (geometry != "" and not data.locations.geometry.has_value())
        data.locations.geometry = geometry;
    
    data.code = Shader::readSPIRV(data.locations);
    data.stages = Shader::createShaderStages(device, data.code);
    
    return data;
}

VK::PipelineCreateInfo VK::preparePipelineCreateInfo(VkExtent2D extent) {
    //---Preprare pipeline info---
    //      Pipelines are huge objects in vulkan, and building them is both complicated and expensive
    //      There is a lot of configuration needed, so this with all the helper functions attempt to break it down into manageable components
    //      Each function explains itself, so refer to them for reference
    PipelineCreateInfo info;
    
    info.vertex_input_binding_description = Vertex::getBindingDescriptionVK<VertexData>();
    info.vertex_input_attribute_descriptions = Vertex::getAttributeDescriptionsVK<VertexData>();
    info.vertex_input = preparePipelineCreateInfoVertexInput(info.vertex_input_binding_description, info.vertex_input_attribute_descriptions);
    
    info.input_assembly = preparePipelineCreateInfoInputAssembly();
    
    info.viewport = preparePipelineCreateInfoViewport(extent);
    info.scissor = preparePipelineCreateInfoScissor(extent);
    info.viewport_state = preparePipelineCreateInfoViewportState(info.viewport, info.scissor);
    
    info.rasterizer = preparePipelineCreateInfoRasterizer();
    
    info.multisampling = preparePipelineCreateInfoMultisampling();
    
    info.depth_stencil = preparePipelineCreateInfoDepthStencil();
    
    info.color_blend_attachment = preparePipelineCreateInfoColorBlendAttachment();
    info.color_blend_state = preparePipelineCreateInfoColorBlendState(info.color_blend_attachment);
    
    return info;
}

VkPipelineVertexInputStateCreateInfo VK::preparePipelineCreateInfoVertexInput(
    const std::vector<VkVertexInputBindingDescription> &binding, const std::vector<VkVertexInputAttributeDescription> &attributes) {
    
    //---Vertex input info---
    //      Each vertex can have a different shape, this struct details it's structure
    //      It has two components, the binding and the attribute descriptions
    //      You can think of it as if the binding is the global binding and size of the struct (we have just one in this case, VertexData)
    //      The attribute description is an array including each member of the array, with its size and stride
    //      We get this information using fresa's reflection on the types we pass
    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    vertex_input.vertexBindingDescriptionCount = (ui32)binding.size();
    vertex_input.pVertexBindingDescriptions = binding.data();
    
    vertex_input.vertexAttributeDescriptionCount = (ui32)attributes.size();
    vertex_input.pVertexAttributeDescriptions = attributes.data();
    
    return vertex_input;
}

VkPipelineInputAssemblyStateCreateInfo VK::preparePipelineCreateInfoInputAssembly() {
    //---Input assembly info---
    //      Here we specify the way the vertices will be processed, in this case a triangle list (Each 3 vertices will form a triangle)
    //      Other possible configurations make it possible to draw points or lines
    //      If primitiveRestartEnable is set to true, it is possible to break strips using a special index, but we won't use it yet
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    
    return input_assembly;
}

VkViewport VK::preparePipelineCreateInfoViewport(VkExtent2D extent) {
    //---Viewport---
    //      The offset and dimensions of the draw viewport inside the window, we just set this to the default
    VkViewport viewport{};
    
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    return viewport;
}

VkRect2D VK::preparePipelineCreateInfoScissor(VkExtent2D extent) {
    //---Scissor---
    //      It is possible to crop the viewport and only present a part of it, but for now we will leave it as default
    VkRect2D scissor{};
    
    scissor.offset = {0, 0};
    scissor.extent = extent;
    
    return scissor;
}

VkPipelineViewportStateCreateInfo VK::preparePipelineCreateInfoViewportState(const VkViewport &viewport, const VkRect2D &scissor) {
    //---Viewport state info---
    //      We combine the viewport and scissor into one vulkan info struct
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;
    
    return viewport_state;
}

VkPipelineRasterizationStateCreateInfo VK::preparePipelineCreateInfoRasterizer() {
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
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    
    //: Line width
    //      Describes the thickness of lines in term of fragments, it requires enabling the wideLines GPU feature
    rasterizer.lineWidth = 1.0f;
    
    //: Culling
    //      We will be culling the back face to save in performance
    //      To calculate the front face, if we are not sending normals, the vertices will be calculated in counter clockwise order
    //      If nothing shows to the screen, one of the most probable causes is the winding order of the vertices to be reversed
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    log::graphics("Culling: back bit - Winding order: CCW");
    
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    
    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VK::preparePipelineCreateInfoMultisampling() {
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

VkPipelineDepthStencilStateCreateInfo VK::preparePipelineCreateInfoDepthStencil() {
    //---Depth info---
    //      WORK IN PROGRESS, get back to this
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    
    depth_stencil.front = {};
    depth_stencil.back = {};
    
    return depth_stencil;
}

VkPipelineColorBlendAttachmentState VK::preparePipelineCreateInfoColorBlendAttachment() {
    //---Color blending attachment---
    //      Specifies how colors should be blended together
    //      We are using alpha blending:
    //          final_color.rgb = new_color.a * new_color.rgb + (1 - new_color.a) * old_color.rgb;
    //          final_color.a = new_color.a;
    //      This still needs testing and possible tweaking
    
    VkPipelineColorBlendAttachmentState color_blend{};
    
    color_blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    color_blend.blendEnable = VK_TRUE;
    
    color_blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend.colorBlendOp = VK_BLEND_OP_ADD;
    
    color_blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend.alphaBlendOp = VK_BLEND_OP_ADD;
    
    return color_blend;
}

VkPipelineColorBlendStateCreateInfo VK::preparePipelineCreateInfoColorBlendState(const VkPipelineColorBlendAttachmentState &attachment) {
    //---Color blending info---
    //      Combines all the color blending attachments into one struct
    //      Note: each framebuffer can have a different attachment, right now we are only using one for all
    //      Here logic operations can be specified manually, but we will use vulkan's blend attachments to handle blending
    
    VkPipelineColorBlendStateCreateInfo color_blend_state{};

    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &attachment;
    
    color_blend_state.blendConstants[0] = 0.0f;
    color_blend_state.blendConstants[1] = 0.0f;
    color_blend_state.blendConstants[2] = 0.0f;
    color_blend_state.blendConstants[3] = 0.0f;
    
    return color_blend_state;
}

VkPipelineLayout VK::createPipelineLayout(VkDevice device, const VkDescriptorSetLayout &descriptor_set_layout) {
    //---Pipeline layout---
    //      Holds the information of the descriptor set layouts that we created earlier
    //      This allows to reference uniforms or images at draw time and change them without recreating the pipeline
    //      TODO: Add support for push constants too
    VkPipelineLayout pipeline_layout;
    
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &descriptor_set_layout;
    
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        log::error("Error creating a vulkan pipeline layout");
    
    deletion_queue_program.push_back([pipeline_layout, device](){ vkDestroyPipelineLayout(device, pipeline_layout, nullptr); });
    log::graphics("Created a vulkan pipeline layout");
    return pipeline_layout;
}

VkPipeline VK::createGraphicsPipeline(VkDevice device, const VkPipelineLayout &layout,
                                      const VkSwapchainData &swapchain, const ShaderStages &stages) {
    //---Pipeline---
    //      The graphics pipeline is a series of stages that convert vertex and other data into a visible image that can be shown to the screen
    //      Input assembler -> Vertex shader -> Tesselation -> Geometry shader -> Rasterization -> Fragment shader -> Color blending -> Frame
    //      Here we put together all the previous helper functions and structs
    //      It holds shader stages, all the creation info, a layout for description sets, render passes...
    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    //: Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> stage_info = Graphics::Shader::getShaderStageInfo(stages);
    create_info.stageCount = (int)stage_info.size();
    create_info.pStages = stage_info.data();
    
    //: Pipeline info
    PipelineCreateInfo pipeline_create_info = preparePipelineCreateInfo(swapchain.extent);
    create_info.pVertexInputState = &pipeline_create_info.vertex_input;
    create_info.pInputAssemblyState = &pipeline_create_info.input_assembly;
    create_info.pViewportState = &pipeline_create_info.viewport_state;
    create_info.pRasterizationState = &pipeline_create_info.rasterizer;
    create_info.pMultisampleState = &pipeline_create_info.multisampling;
    create_info.pDepthStencilState = &pipeline_create_info.depth_stencil;
    create_info.pColorBlendState = &pipeline_create_info.color_blend_state;
    create_info.pDynamicState = nullptr;
    create_info.pTessellationState = nullptr;
    
    //: Layout
    create_info.layout = layout;
    
    //: Render pass
    create_info.renderPass = swapchain.main_render_pass;
    create_info.subpass = 0;
    
    //: Recreation info
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;
    
    //---Create pipeline---
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        log::error("Error while creating a vulkan graphics pipeline");
    
    deletion_queue_swapchain.push_back([pipeline, device](){ vkDestroyPipeline(device, pipeline, nullptr); });
    log::graphics("Created a vulkan graphics pipeline");
    return pipeline;
}

//----------------------------------------



//Buffers
//----------------------------------------

BufferData VK::createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory) {
    //---Buffers---
    //      These are regions of memory that store arbitrary data that the CPU, GPU or both can read
    //      We are using Vulkan Memory Allocator for allocating their memory in a bigger pool
    BufferData data;
    
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocate_info{};
    allocate_info.usage = memory;
    
    if (vmaCreateBuffer(allocator, &create_info, &allocate_info, &data.buffer, &data.allocation, nullptr) != VK_SUCCESS)
        log::error("Failed to create a vulkan buffer");
    
    return data;
}

BufferData VK::createVertexBuffer(VkDevice device, VmaAllocator allocator,
                                  const VkCommandData &cmd, const std::vector<Graphics::VertexData> &vertices) {
    //---Vertex buffer---
    //      Buffer that holds the vertex information for the shaders to use.
    //      It has a VertexData struct per vertex of the mesh, which can contain properties like position, color, uv, normals...
    //      The properties are described automatically using reflection in an attribute description in the pipeline
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    //: Staging buffer
    //      We want to make the vertex buffer accessible to the GPU in the most efficient way, so we use a staging buffer
    //      This is created in CPU only memory, in which we can easily map the vertex data
    BufferData staging_buffer = createBuffer(allocator, buffer_size,
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VMA_MEMORY_USAGE_CPU_ONLY);
    
    //: Map vertices to staging buffer
    void* data;
    vmaMapMemory(allocator, staging_buffer.allocation, &data);
    memcpy(data, vertices.data(), (size_t) buffer_size);
    vmaUnmapMemory(allocator, staging_buffer.allocation);
    
    //: Vertex buffer
    //      The most efficient memory for GPU access is VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, or its rough equivalent, VMA_MEMORY_USAGE_GPU_ONLY
    //      This memory type can't be access from the CPU
    BufferData vertex_buffer = createBuffer(allocator, buffer_size,
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);
    
    //: Copy from staging to vertex
    //      Since we can't access the vertex buffer memory from the CPU, we will use vkCmdCopyBuffer(), which will execute on a queue
    //      and move data between the staging and vertex buffer
    copyBuffer(device, cmd, staging_buffer.buffer, vertex_buffer.buffer, buffer_size);
    
    //: Delete helpers (staging now, vertex when the program finishes)
    vmaDestroyBuffer(allocator, staging_buffer.buffer, staging_buffer.allocation);
    deletion_queue_program.push_back([allocator, vertex_buffer](){
        vmaDestroyBuffer(allocator, vertex_buffer.buffer, vertex_buffer.allocation);
    });
    
    return vertex_buffer;
}

BufferData VK::createIndexBuffer(VkDevice device, VmaAllocator allocator,
                                 const VkCommandData &cmd, const std::vector<ui16> &indices) {
    //---Index buffer---
    //      This buffer contains a list of indices, which allows to draw complex meshes without repeating vertices
    //      A simple example, while a square only has 4 vertices, 6 vertices are needed for the 2 triangles, and it only gets worse from there
    //      An index buffer solves this by having a list of which vertices to use, avoiding vertex repetition
    //      The creating process is very similar to the above vertex buffer, using a staging buffer
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    //: Staging buffer
    BufferData staging_buffer = createBuffer(allocator, buffer_size,
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VMA_MEMORY_USAGE_CPU_ONLY);
    
    //: Map indices to staging buffer
    void* data;
    vmaMapMemory(allocator, staging_buffer.allocation, &data);
    memcpy(data, indices.data(), (size_t) buffer_size);
    vmaUnmapMemory(allocator, staging_buffer.allocation);
    
    //: Index buffer
    BufferData index_buffer = createBuffer(allocator, buffer_size,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VMA_MEMORY_USAGE_GPU_ONLY);
    
    //: Copy from staging to index
    copyBuffer(device, cmd, staging_buffer.buffer, index_buffer.buffer, buffer_size);
    
    //: Delete helpers (staging now, index when the program finishes)
    vmaDestroyBuffer(allocator, staging_buffer.buffer, staging_buffer.allocation);
    deletion_queue_program.push_back([allocator, index_buffer](){
        vmaDestroyBuffer(allocator, index_buffer.buffer, index_buffer.allocation);
    });
    
    return index_buffer;
}

void VK::copyBuffer(VkDevice device, const VkCommandData &cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    //---Copy buffer---
    //      Simple function that creates a command buffer to copy the memory from one buffer to another
    //      Very helpful when using staging buffers to move information from CPU to GPU only memory (can be done in reverse)
    VkCommandBuffer command_buffer = beginSingleUseCommandBuffer(device, cmd.command_pools.at("transfer"));
    
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);
    
    endSingleUseCommandBuffer(device, command_buffer, cmd.command_pools.at("transfer"), cmd.queues.transfer);
}

//----------------------------------------



//Descriptors
//----------------------------------------

VkDescriptorSetLayoutBinding VK::prepareDescriptorSetLayoutBinding(VkShaderStageFlagBits stage, VkDescriptorType type, ui32 binding) {
    //---Descriptor set layout binding---
    //      One item of a descriptor set layout, it includes the stage (vertex, frag...),
    //      the type of the descriptor (uniform, image...) and the binding position
    VkDescriptorSetLayoutBinding layout_binding;
    
    layout_binding.binding = binding;
    layout_binding.descriptorType = type;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = stage;
    layout_binding.pImmutableSamplers = nullptr;
    
    return layout_binding;
}

VkDescriptorSetLayout VK::createDescriptorSetLayout(VkDevice device, const ShaderCode &code, ui32 swapchain_size) {
    //---Descriptor set layout---
    //      It is a blueprint for creating descriptor sets, specifies the number and type of descriptors in the GLSL shader
    //      Descriptors can be anything passed into a shader: uniforms, images, ...
    //      We use SPIRV-cross to get reflection from the shaders themselves and create the descriptor layout automatically
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSetLayoutBinding> layout_binding;
    
    log::graphics("");
    log::graphics("Creating descriptor set layout...");
    
    ui32 uniform_count = 0;
    ui32 image_sampler_count = 0;
    
    //---Vertex shader---
    if (code.vert.has_value()) {
        ShaderCompiler compiler = Shader::getShaderCompiler(code.vert.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers) {
            ui32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            log::graphics(" - Uniform buffer (%s) - Binding : %d - Stage: Vertex", res.name.c_str(), binding);
            layout_binding.push_back(
                prepareDescriptorSetLayoutBinding(VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding));
            uniform_count++;
        }
    }
    
    //---Fragment shader---
    if (code.frag.has_value()) {
        ShaderCompiler compiler = Shader::getShaderCompiler(code.frag.value());
        ShaderResources resources = compiler.get_shader_resources();
        
        //: Uniform buffers
        for (const auto &res : resources.uniform_buffers) {
            ui32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            log::graphics(" - Uniform buffer (%s) - Binding : %d - Stage: Fragment", res.name.c_str(), binding);
            layout_binding.push_back(
                prepareDescriptorSetLayoutBinding(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding));
            uniform_count++;
        }
        
        //: Combined image samplers
        for (const auto &res : resources.sampled_images) {
            ui32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            log::graphics(" - Image (%s) - Binding : %d - Stage : Fragment", res.name.c_str(), binding);
            layout_binding.push_back(
                prepareDescriptorSetLayoutBinding(VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding));
            image_sampler_count++;
        }
    }
    
    //---Define the descriptor pool sizes---
    //      This specifies how many resources of each type we need, and will be later used when creating a descriptor pool
    //      We use the number of items of that type, times the number of images in the swapchain
    descriptor_pool_sizes.clear();
    if (uniform_count > 0)
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform_count * swapchain_size});
    if (image_sampler_count > 0)
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, image_sampler_count * swapchain_size});
    
    //---Create the descriptor layout itself---
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = (ui32)layout_binding.size();
    create_info.pBindings = layout_binding.data();
    
    if (vkCreateDescriptorSetLayout(device, &create_info, nullptr, &layout) != VK_SUCCESS)
        log::error("Error creating a descriptor set layout, check shader refraction");
    
    deletion_queue_program.push_back([device, layout](){ vkDestroyDescriptorSetLayout(device, layout, nullptr); });
    return layout;
}

VkDescriptorPool VK::createDescriptorPool(VkDevice device) {
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
    //      This uses the descriptor pool size from the list of sizes initialized in createDescriptorSetLayout() using SPIRV reflection
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = (ui32)descriptor_pool_sizes.size();
    create_info.pPoolSizes = descriptor_pool_sizes.data();
    create_info.maxSets = std::accumulate(descriptor_pool_sizes.begin(), descriptor_pool_sizes.end(), 0,
                                          [](ui32 sum, const VkDescriptorPoolSize &item){ return sum + item.descriptorCount; });
    
    //: Create descriptor pool
    if (vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool) != VK_SUCCESS)
        log::error("Failed to create a vulkan descriptor pool");
    
    //: Log the allowed types and respective sizes
    log::graphics("");
    log::graphics("Created a vulkan descriptor pool with sizes:");
    for (const auto &item : descriptor_pool_sizes) {
        str name = [item](){
            switch (item.type) {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    return "Uniform";
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    return "Image sampler";
                default:
                    return "";
            }
        }();
        log::graphics(" - %s: %d", name.c_str(), item.descriptorCount);
    }
    log::graphics("");
    
    deletion_queue_program.push_back([device, descriptor_pool](){ vkDestroyDescriptorPool(device, descriptor_pool, nullptr); });
    return descriptor_pool;
}

std::vector<VkDescriptorSet> VK::allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout,
                                                        VkDescriptorPool pool, ui32 swapchain_size) {
    //---Descriptor sets---
    //      These objects can be thought as a pointer to a resource that the shader can access, for example an image sampler or uniform bufffer
    //      We allocate one set per swapchain image since multiple frames can be in flight at the same time
    std::vector<VkDescriptorSet> descriptor_sets(swapchain_size);
    
    //: Allocate information
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = pool;
    allocate_info.descriptorSetCount = swapchain_size;
    std::vector<VkDescriptorSetLayout> layouts(swapchain_size, layout);
    allocate_info.pSetLayouts = layouts.data();
    
    //: Create the descriptor set
    if (vkAllocateDescriptorSets(device, &allocate_info, descriptor_sets.data()) != VK_SUCCESS)
        log::error("Failed to allocate vulkan descriptor sets");
    
    //: No need for cleanup since they get destroyed with the pool
    log::graphics("Allocated vulkan descriptor sets");
    return descriptor_sets;
}

VK::WriteDescriptorBuffer VK::createWriteDescriptorUniformBuffer(VkDescriptorSet descriptor_set, ui32 binding, BufferData uniform_buffer) {
    WriteDescriptorBuffer write_descriptor{};
    
    write_descriptor.info.buffer = uniform_buffer.buffer;
    write_descriptor.info.offset = 0;
    write_descriptor.info.range = VK_WHOLE_SIZE;
    
    write_descriptor.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor.write.dstSet = descriptor_set;
    write_descriptor.write.dstBinding = binding;
    write_descriptor.write.dstArrayElement = 0;
    write_descriptor.write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor.write.descriptorCount = 1;
    write_descriptor.write.pBufferInfo = &write_descriptor.info;
    write_descriptor.write.pImageInfo = nullptr;
    write_descriptor.write.pTexelBufferView = nullptr;
    
    return write_descriptor;
}

VK::WriteDescriptorImage VK::createWriteDescriptorCombinedImageSampler(VkDescriptorSet descriptor_set, ui32 binding,
                                                                       VkImageView image_view, VkSampler sampler) {
    WriteDescriptorImage write_descriptor{};
    
    write_descriptor.info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    write_descriptor.info.imageView = image_view;
    write_descriptor.info.sampler = sampler;
    
    write_descriptor.write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor.write.dstSet = descriptor_set;
    write_descriptor.write.dstBinding = binding;
    write_descriptor.write.dstArrayElement = 0;
    write_descriptor.write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor.write.descriptorCount = 1;
    write_descriptor.write.pBufferInfo = nullptr;
    write_descriptor.write.pImageInfo = &write_descriptor.info;
    write_descriptor.write.pTexelBufferView = nullptr;
    
    return write_descriptor;
}

void VK::updateDescriptorSets(VkDevice device, const std::vector<VkDescriptorSet> &descriptor_sets, ui32 swapchain_size,
                              const std::vector<BufferData> &uniform_buffers) {
    for (int i = 0; i < swapchain_size; i++) {
        std::vector<VkWriteDescriptorSet> write_descriptors;
        
        auto uniform_write_descriptor = createWriteDescriptorUniformBuffer(descriptor_sets[i], 0, uniform_buffers[i]);
        write_descriptors.push_back(uniform_write_descriptor.write);
        
        //auto image_sampler_write_descriptor = createWriteDescriptorCombinedImageSampler(descriptor_sets[i], 1, image_view[i], sampler);
        //write_descriptors.push_back(image_sampler_write_descriptor.write);
        
        vkUpdateDescriptorSets(device, (ui32)write_descriptors.size(), write_descriptors.data(), 0, nullptr);
    }
    
    log::graphics("Updated descriptor sets");
}

//----------------------------------------



//Uniforms
//----------------------------------------

std::vector<BufferData> VK::createUniformBuffers(VmaAllocator allocator, ui32 swapchain_size) {
    //---Uniform buffers---
    //      Appart from per vertex data, we might want to pass "constants" to a shader program
    //      An example of this could be projection, view and model matrices that can be used as a camera
    //      We do this using uniform buffers, that we need to create descriptor sets for, and the pass them to the shader
    //      The creation process is identical to other buffers, but we want them to have memory accessible to both the CPU and GPU
    //      This way we can update them every frame with new information
    //      We need to have one buffer per swapchain image so we can work on multiple images at a time
    //      Another possibility for passing data to shaders are push constant, which we will implement in the future
    std::vector<BufferData> uniform_buffers;
    VkDeviceSize buffer_size = sizeof(VK::UniformBufferObject);
    
    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VmaMemoryUsage memory_flags = VMA_MEMORY_USAGE_CPU_TO_GPU;
    for (int i = 0; i < swapchain_size; i++)
        uniform_buffers.push_back(createBuffer(allocator, buffer_size, usage_flags, memory_flags));
    
    deletion_queue_program.push_back([allocator, uniform_buffers](){
        for (const auto &buffer : uniform_buffers)
            vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    });
    log::graphics("Created vulkan uniform buffers");
    return uniform_buffers;
}

//----------------------------------------



//Images
//----------------------------------------

void API::createTexture(Vulkan &vk, TextureData &tex, ui8 *pixels) {
    /*ui32 size = tex.w * tex.h * tex.ch * sizeof(ui8);
    
    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    BufferData staging_buffer = VK::createBuffer(vk, size, usage_flags, memory_flags);
    
    void* data;
    vkMapMemory(vk.device, staging_buffer.memory, 0, size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(vk.device, staging_buffer.memory);
    
    VK::createImage(vk, tex, pixels);
    VK::transitionImageLayout(vk, tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VK::copyBufferToImage(vk, staging_buffer, tex);
    VK::transitionImageLayout(vk, tex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vk.image_view = VK::createImageView(vk.device, tex.image, VK_IMAGE_ASPECT_COLOR_BIT, tex.format);
    
    vkDestroyBuffer(vk.device, staging_buffer.buffer, nullptr);
    vkFreeMemory(vk.device, staging_buffer.memory, nullptr);*/
}

void VK::createImage(Vulkan &vk, TextureData &tex, ui8 *pixels) {
    /*VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = static_cast<ui32>(tex.w);
    create_info.extent.height = static_cast<ui32>(tex.h);
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    
    //This way the image can't be access directly, but it is better for performance
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    
    tex.format = [tex](){
        switch(tex.ch) {
            case 1:
                return VK_FORMAT_R8_SRGB;
            case 2:
                return VK_FORMAT_R8G8_SRGB;
            case 3:
                return VK_FORMAT_R8G8B8_SRGB;
            default:
                return VK_FORMAT_R8G8B8A8_SRGB;
        }
        //Is it necessary to test for alternatives?
    }();
    create_info.format = tex.format;
    
    tex.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.initialLayout = tex.layout;
    
    create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.flags = 0;
    
    if (vkCreateImage(vk.device, &create_info, nullptr, &tex.image) != VK_SUCCESS)
        log::error("Failed to create a Vulkan image");
    
    
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(vk.device, tex.image, &memory_requirements);
    
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = getMemoryType(vk, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(vk.device, &allocate_info, nullptr, &tex.memory) != VK_SUCCESS)
        log::error("Failed to allocate memory for a Vulkan image");
    
    vkBindImageMemory(vk.device, tex.image, tex.memory, 0);*/
}

void VK::transitionImageLayout(Vulkan &vk, TextureData &tex, VkImageLayout new_layout) {
    /*VkCommandBuffer command_buffer = beginSingleUseCommandBuffer(vk.device, vk.cmd.command_pools.at("temp"));

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = tex.image;
    
    barrier.oldLayout = tex.layout;
    barrier.newLayout = new_layout;
    
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;
    
    barrier.srcAccessMask = [tex, &src_stage](){
        switch (tex.layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                return (VkAccessFlagBits)0;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            default:
                log::warn("Not a valid src access mask in image layout transition");
                return (VkAccessFlagBits)0;
        }
    }();
    
    barrier.dstAccessMask = [new_layout, &dst_stage](){
        switch (new_layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                return VK_ACCESS_TRANSFER_WRITE_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                return VK_ACCESS_SHADER_READ_BIT;
            default:
                log::warn("Not a valid dst access mask in image layout transition");
                return (VkAccessFlagBits)0;
        }
    }();
    
    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier); //TODO
    
    endSingleUseCommandBuffer(vk.device, command_buffer, vk.cmd.command_pools.at("temp"), vk.cmd.queues.graphics);
    
    tex.layout = new_layout;*/
}

void VK::copyBufferToImage(Vulkan &vk, BufferData &buffer, TextureData &tex) {
    /*VkCommandBuffer command_buffer = beginSingleUseCommandBuffer(vk.device, vk.cmd.command_pools.at("transfer"));
    
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
    
    endSingleUseCommandBuffer(vk.device, command_buffer, vk.cmd.command_pools.at("transfer"), vk.cmd.queues.transfer);*/
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
        log::error("Error creating a Vulkan image view");
    
    return image_view;
}

void VK::createSampler(Vulkan &vk) {
    /*VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    create_info.magFilter = VK_FILTER_NEAREST; //This is for pixel art, change to linear for interpolation
    create_info.minFilter = VK_FILTER_NEAREST;
    
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; //The texture doesn't repeat if sampled out of range
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    
    create_info.anisotropyEnable = VK_FALSE;
    create_info.maxAnisotropy = 1.0f;*/
    
    //Enable for anisotropy, as well as device features in createDevice()
    /*VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vk.physical_device, &properties);
    create_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;*/
    
    /*create_info.unnormalizedCoordinates = VK_FALSE;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0f;
    create_info.maxLod = 0.0f;
    
    if (vkCreateSampler(vk.device, &create_info, nullptr, &vk.sampler) != VK_SUCCESS)
        log::error("Error creating a Vulkan image sampler");*/
}

//----------------------------------------



//Render
//----------------------------------------

ui32 VK::startRender(VkDevice device, const VkSwapchainData &swapchain, VkSyncData &sync) {
    //---Start rendering---
    //      TODO: .
    ui32 image_index;
    
    vkWaitForFences(device, 1, &sync.fences_in_flight[sync.current_frame], VK_TRUE, UINT64_MAX);
    
    VkResult result = vkAcquireNextImageKHR(device, swapchain.swapchain, UINT64_MAX,
                                            sync.semaphores_image_available[sync.current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //TODO: recreateSwapchain(vk, win);
        return UINT32_MAX;
    }
    
    if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR)
        log::error("Failed to acquire swapchain image");
    
    if (sync.fences_images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &sync.fences_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    
    sync.fences_images_in_flight[image_index] = sync.fences_in_flight[sync.current_frame];
    
    return image_index;
}

void VK::renderFrame(Vulkan &vk, WindowData &win, ui32 index) {
    VkSemaphore wait_semaphores[]{ vk.sync.semaphores_image_available[vk.sync.current_frame] };
    VkPipelineStageFlags wait_stages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[]{ vk.sync.semaphores_render_finished[vk.sync.current_frame] };
    
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk.cmd.command_buffers["draw"][index];
    
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    
    vkResetFences(vk.device, 1, &vk.sync.fences_in_flight[vk.sync.current_frame]);
    
    if (vkQueueSubmit(vk.cmd.queues.graphics, 1, &submit_info, vk.sync.fences_in_flight[vk.sync.current_frame]) != VK_SUCCESS)
        log::error("Failed to submit Draw Command Buffer");
    
    
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[]{ vk.swapchain.swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &index;
    present_info.pResults = nullptr;
    
    
    VkResult result = vkQueuePresentKHR(vk.cmd.queues.present, &present_info);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
        recreateSwapchain(vk, win);
    else if (result != VK_SUCCESS)
        log::error("Failed to present Swapchain Image");
    
    
    vk.sync.current_frame = (vk.sync.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//----------------------------------------


//Test
//----------------------------------------

void API::renderTest(WindowData &win, RenderData &render) {
    //---Example update---
    //:
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    VK::UniformBufferObject ubo{};
    
    ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    ubo.model = glm::rotate(ubo.model, t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, 0.3f * std::sin(t * 1.570796f)));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), win.size.x / (float) win.size.y, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    //:
    
    ui32 index = VK::startRender(render.api.device, render.api.swapchain, render.api.sync);
    
    VK::updateUniformBuffer(render.api.allocator, render.api.uniform_buffers[index], ubo);
    
    VK::renderFrame(render.api, win, index);
}

//----------------------------------------



//Resize
//----------------------------------------

void API::resize(Vulkan &vk, WindowData &win) {
    VK::recreateSwapchain(vk, win);
}

//----------------------------------------



//Cleanup
//----------------------------------------

void API::clean(Vulkan &vk) {
    vkDeviceWaitIdle(vk.device);
    
    //: Delete swapchain objects
    for (auto it = deletion_queue_swapchain.rbegin(); it != deletion_queue_swapchain.rend(); ++it)
        (*it)();
    
    //: Delete objects that depend on swapchain size
    deletion_queue_swapchain.clear();
    for (auto it = deletion_queue_swapchain_extra.rbegin(); it != deletion_queue_swapchain_extra.rend(); ++it)
        (*it)();
    deletion_queue_swapchain_extra.clear();
    
    //: Delete shader stages
    Shader::destroyShaderStages(vk.device, vk.shader.stages);
    
    //TODO: MOVE
    vkDestroyImageView(vk.device, vk.image_view, nullptr);
    vkDestroyImage(vk.device, vk.test_image.image, nullptr);
    vkFreeMemory(vk.device, vk.test_image.memory, nullptr);
    vkDestroySampler(vk.device, vk.sampler, nullptr);
    
    //: Delete program resources
    for (auto it = deletion_queue_program.rbegin(); it != deletion_queue_program.rend(); ++it)
        (*it)();
    deletion_queue_program.clear();
    
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

#endif
