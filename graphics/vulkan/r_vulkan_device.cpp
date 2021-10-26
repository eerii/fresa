//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

#include <set>

using namespace Verse;
using namespace Graphics;

namespace
{
    const std::vector<const char*> validation_layers{
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> required_device_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

void Vulkan::createInstance(WindowData &win) {
    log::graphics("");
    
    ui32 extension_count;
    SDL_Vulkan_GetInstanceExtensions(win.window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(win.window, &extension_count, extension_names.data());
    log::graphics("Vulkan requested Instance Extensions: %d", extension_count);
    for (const char* ext : extension_names)
        log::graphics(" - %s", ext);
    
    log::graphics("");
    
    ui32 validation_layer_count;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
    log::graphics("Vulkan supported Validation Layers: %d", validation_layer_count);
    for (VkLayerProperties layer : available_validation_layers)
        log::graphics(" - %s", layer.layerName);
    
    log::graphics("");
    
    for (const char* val : validation_layers) {
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
    
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = Conf::name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(Conf::version[0], Conf::version[1], Conf::version[2]);
    app_info.pEngineName = "Fresa";
    app_info.engineVersion = VK_MAKE_VERSION(Conf::version[0], Conf::version[1], Conf::version[2]);
    app_info.apiVersion = VK_API_VERSION_1_1;
    
    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = (int)validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
    instance_create_info.enabledExtensionCount = (int)extension_names.size();
    instance_create_info.ppEnabledExtensionNames = extension_names.data();
    
    if (vkCreateInstance(&instance_create_info, nullptr, &instance)!= VK_SUCCESS)
        log::error("Error creating Vulkan Instance");
}

ui16 Vulkan::ratePhysicalDevice(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);
    
    ui16 score = 16;
    
    //Properties
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 256;
    
    //Queues
    VK::QueueFamilyIndices queue_indices = getQueueFamilies(physical_device);
    if (queue_indices.compute_queue_family_index.has_value())
        score += 16;
    if (not queue_indices.present_queue_family_index.has_value())
        return 0;
    if (not queue_indices.graphics_queue_family_index.has_value())
        return 0;
    
    //Extensions
    ui32 extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());
    
    std::set<std::string> required_extensions(required_device_extensions.begin(), required_device_extensions.end());
    
    for (VkExtensionProperties extension : available_extensions)
        required_extensions.erase(extension.extensionName);

    if (not required_extensions.empty())
        return 0;
    
    //Swapchain
    VK::SwapchainSupportDetails details = getSwapchainSupport(physical_device);
    if (details.formats.empty() or details.present_modes.empty())
        return 0;
    
    return score;
}

void Vulkan::selectPhysicalDevice() {
    ui32 device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    
    if (device_count == 0)
        log::error("There are no GPUs with Vulkan Support!");
    
    log::graphics("Vulkan required Device Extensions: %d", required_device_extensions.size());
    for (const char* ext : required_device_extensions)
        log::graphics(" - %s", ext);
    log::graphics("");
    
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    log::graphics("Vulkan Physical Devices: %d", device_count);
    
    std::multimap<VkPhysicalDevice, ui16> candidates;
    for (VkPhysicalDevice dev : devices)
        candidates.insert(std::make_pair(dev, ratePhysicalDevice(dev)));
    
    if (candidates.rbegin()->second > 0)
        physical_device = candidates.rbegin()->first;
    
    for (VkPhysicalDevice dev : devices) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(dev, &device_properties);
        log::graphics(((dev == physical_device) ? " > %s" : " - %s"), device_properties.deviceName);
    }
    
    log::graphics("");
    
    if (physical_device == VK_NULL_HANDLE)
        log::error("No GPU passed the Vulkan Physical Device Requirements.");
}

VK::QueueFamilyIndices Vulkan::getQueueFamilies(VkPhysicalDevice physical_device) {
    ui32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_list.data());
    
    VK::QueueFamilyIndices indices;
    
    for (int i = 0; i < queue_family_list.size(); i++) {
        if (not indices.present_queue_family_index.has_value()) {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
            if(present_support)
                indices.present_queue_family_index = i;
        }
        
        if (not indices.graphics_queue_family_index.has_value() and (queue_family_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphics_queue_family_index = i;
            continue;
        }
        
        if (not indices.compute_queue_family_index.has_value() and (queue_family_list[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.compute_queue_family_index = i;
            continue;
        }
        
        if (indices.all())
            break;
    }
    
    return indices;
}

void Vulkan::selectQueueFamily() {
    queue_families = getQueueFamilies(physical_device);
}

void Vulkan::createDevice() {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    
    std::set<ui32> unique_queue_families{};
    if (queue_families.graphics_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.graphics_queue_family_index.value());
    if (queue_families.present_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.present_queue_family_index.value());
    if (queue_families.compute_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.compute_queue_family_index.value());
    
    int i = 0;
    std::vector<float> priorities{ 1.0f, 1.0f, 0.5f };
    
    for (ui32 family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_graphics_info{};
        queue_graphics_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_graphics_info.queueFamilyIndex = family;
        queue_graphics_info.queueCount = 1;
        queue_graphics_info.pQueuePriorities = &priorities[i];
        queue_create_infos.push_back(queue_graphics_info);
        i++;
    }
    
    //Device required features
    VkPhysicalDeviceFeatures device_features{};
    
    VkDeviceCreateInfo device_create_info{};
    
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = (int)queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledExtensionCount = (int)required_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.pEnabledFeatures = &device_features;
    
    device_create_info.enabledLayerCount = (int)validation_layers.size();
    device_create_info.ppEnabledLayerNames = validation_layers.data();
    
    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device)!= VK_SUCCESS)
        log::error("Error creating Vulkan Logical Device");
    
    log::graphics("Vulkan Queue Families: %d", unique_queue_families.size());
    if (queue_families.graphics_queue_family_index.has_value()) {
        vkGetDeviceQueue(device, queue_families.graphics_queue_family_index.value(), 0, &graphics_queue);
        log::graphics(" - Graphics (%d)", queue_families.graphics_queue_family_index.value());
    }
    if (queue_families.present_queue_family_index.has_value()) {
        vkGetDeviceQueue(device, queue_families.present_queue_family_index.value(), 0, &present_queue);
        log::graphics(" - Present (%d)", queue_families.present_queue_family_index.value());
    }
    if (queue_families.compute_queue_family_index.has_value()) {
        vkGetDeviceQueue(device, queue_families.compute_queue_family_index.value(), 0, &compute_queue);
        log::graphics(" - Compute (%d)", queue_families.compute_queue_family_index.value());
    }
    log::graphics("");
}

void Vulkan::createSurface(WindowData &win) {
    if (not SDL_Vulkan_CreateSurface(win.window, instance, &surface))
        log::error("Error while creating a Vulkan Surface (from createSurface): %s", SDL_GetError());
}

#endif
