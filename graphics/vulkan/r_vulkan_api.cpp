//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_api.h"
#include "r_api.h"

#include "ftime.h"
#include "r_shader.h"

#include <set>
#include <glm/gtc/matrix_transform.hpp>

using namespace Verse;
using namespace Graphics;

namespace {
    const std::vector<Graphics::VertexData> vertices = {
        {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f}, {0.7f, 0.3f, 0.7f}},
    };

    const std::vector<ui16> indices = {
        0, 1, 2, 2, 3, 0
    };

    const std::vector<const char*> validation_layers{
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> required_device_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}



//INITIALIZATION
//----------------------------------------

void API::configure() {
    
}

VulkanOld API::create(WindowData &win) {
    VulkanOld vulkan;
    
    vulkan.createInstance(win);
    vulkan.createDebug();
    
    vulkan.createSurface(win);
    
    vulkan.selectPhysicalDevice();
    vulkan.selectQueueFamily();
    vulkan.createDevice();
    
    vulkan.createSwapchain(win);
    vulkan.createImageViews();
    
    vulkan.createRenderPass();
    vulkan.createDescriptorSetLayout();
    vulkan.createGraphicsPipeline();
    
    vulkan.createFramebuffers();
    vulkan.createCommandPools();
    vulkan.createVertexBuffer(vertices);
    vulkan.createIndexBuffer(indices);
    
    vulkan.createUniformBuffers();
    vulkan.createDescriptorPool();
    vulkan.createDescriptorSets();
    
    vulkan.createCommandBuffers();
    
    vulkan.createSyncObjects();
    
    return vulkan;
}

//----------------------------------------



//DEVICE
//----------------------------------------

void VulkanOld::createInstance(WindowData &win) {
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

ui16 VulkanOld::ratePhysicalDevice(VkPhysicalDevice physical_device) {
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

void VulkanOld::selectPhysicalDevice() {
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

VK::QueueFamilyIndices VulkanOld::getQueueFamilies(VkPhysicalDevice physical_device) {
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

void VulkanOld::selectQueueFamily() {
    queue_families = getQueueFamilies(physical_device);
}

void VulkanOld::createDevice() {
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

void VulkanOld::createSurface(WindowData &win) {
    if (not SDL_Vulkan_CreateSurface(win.window, instance, &surface))
        log::error("Error while creating a Vulkan Surface (from createSurface): %s", SDL_GetError());
}

//----------------------------------------



//SWAPCHAIN
//----------------------------------------

VK::SwapchainSupportDetails VulkanOld::getSwapchainSupport(VkPhysicalDevice physical_device) {
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

VkSurfaceFormatKHR VulkanOld::selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (VkSurfaceFormatKHR fmt : available_formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return fmt;
    }
    return available_formats[0];
}

VkPresentModeKHR VulkanOld::selectSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes) {
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

VkExtent2D VulkanOld::selectSwapExtent(WindowData &win, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(win.window, &w, &h);
    
    VkExtent2D actual_extent{ static_cast<ui32>(w), static_cast<ui32>(h) };
    
    std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actual_extent;
}

void VulkanOld::createSwapchain(WindowData &win) {
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

void VulkanOld::createImageViews() {
    swapchain_image_views.resize(swapchain_images.size());
    
    for (int i = 0; i < swapchain_images.size(); i++)
        swapchain_image_views[i] = createImageView(swapchain_images[i], VK_IMAGE_ASPECT_COLOR_BIT);
    
    log::graphics("Created all Vulkan Image Views");
}

VkImageView VulkanOld::createImageView(VkImage image, VkImageAspectFlags aspect_flags) {
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

void VulkanOld::recreateSwapchain(WindowData &win) {
    vkDeviceWaitIdle(device);
    
    cleanSwapchain();
    
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

//----------------------------------------



//PIPELINE
//----------------------------------------

void VulkanOld::createRenderPass() {
    VkAttachmentDescription color_attachment{};
    
    color_attachment.format = swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    
    VkSubpassDescription subpass = createRenderSubpass();
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    if (vkCreateRenderPass(device, &create_info, nullptr, &render_pass) != VK_SUCCESS)
        log::error("Error creating a Vulkan Render Pass");
    
    log::graphics("Created all Vulkan Render Passes");
}

VkSubpassDescription VulkanOld::createRenderSubpass() {
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    
    return subpass;
}

void VulkanOld::prepareRenderInfo() {
    prepareRenderInfoVertexInput();
    prepareRenderInfoInputAssembly();
    prepareRenderInfoViewportState();
    prepareRenderInfoRasterizer();
    prepareRenderInfoMultisampling();
    prepareRenderInfoDepthStencil();
    prepareRenderInfoColorBlendAttachment();
    prepareRenderInfoColorBlendState();
}

void VulkanOld::prepareRenderInfoVertexInput() {
    rendering_create_info.vertex_input = {};
    rendering_create_info.vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    rendering_create_info.vertex_input_binding_description = Vertex::getBindingDescriptionVK();
    rendering_create_info.vertex_input_attribute_descriptions = Vertex::getAttributeDescriptionsVK<VertexData>();
    
    rendering_create_info.vertex_input.vertexBindingDescriptionCount = 1;
    rendering_create_info.vertex_input.pVertexBindingDescriptions = &rendering_create_info.vertex_input_binding_description;
    rendering_create_info.vertex_input.vertexAttributeDescriptionCount = static_cast<ui32>(rendering_create_info.vertex_input_attribute_descriptions.size());
    rendering_create_info.vertex_input.pVertexAttributeDescriptions = rendering_create_info.vertex_input_attribute_descriptions.data();
}

void VulkanOld::prepareRenderInfoInputAssembly() {
    rendering_create_info.input_assembly = {};
    
    rendering_create_info.input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    rendering_create_info.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    rendering_create_info.input_assembly.primitiveRestartEnable = VK_FALSE; //Change when using an index buffer
}

void VulkanOld::prepareRenderInfoViewportState() {
    rendering_create_info.viewport = {};
    rendering_create_info.viewport.x = 0.0f;
    rendering_create_info.viewport.y = 0.0f;
    rendering_create_info.viewport.width = (float)swapchain_extent.width;
    rendering_create_info.viewport.height = (float)swapchain_extent.height;
    rendering_create_info.viewport.minDepth = 0.0f;
    rendering_create_info.viewport.maxDepth = 1.0f;
    
    rendering_create_info.scissor = {};
    rendering_create_info.scissor.offset = {0, 0};
    rendering_create_info.scissor.extent = swapchain_extent;
    
    rendering_create_info.viewport_state = {};
    
    rendering_create_info.viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    rendering_create_info.viewport_state.viewportCount = 1;
    rendering_create_info.viewport_state.pViewports = &rendering_create_info.viewport;
    rendering_create_info.viewport_state.scissorCount = 1;
    rendering_create_info.viewport_state.pScissors = &rendering_create_info.scissor;
}

void VulkanOld::prepareRenderInfoRasterizer() {
    rendering_create_info.rasterizer = {};
    
    rendering_create_info.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rendering_create_info.rasterizer.depthClampEnable = VK_FALSE;
    rendering_create_info.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    
    rendering_create_info.rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Fill, Line, Point, requires enabling GPU features
    rendering_create_info.rasterizer.lineWidth = 1.0f; //Larger thickness requires enabling GPU features
    
    rendering_create_info.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rendering_create_info.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    rendering_create_info.rasterizer.depthBiasEnable = VK_FALSE;
    rendering_create_info.rasterizer.depthBiasConstantFactor = 0.0f;
    rendering_create_info.rasterizer.depthBiasClamp = 0.0f;
    rendering_create_info.rasterizer.depthBiasSlopeFactor = 0.0f;
}

void VulkanOld::prepareRenderInfoMultisampling() {
    rendering_create_info.multisampling = {};
    
    rendering_create_info.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    rendering_create_info.multisampling.sampleShadingEnable = VK_FALSE; //Needs to be enabled in the future
    
    rendering_create_info.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    rendering_create_info.multisampling.minSampleShading = 1.0f;
    rendering_create_info.multisampling.pSampleMask = nullptr;
    
    rendering_create_info.multisampling.alphaToCoverageEnable = VK_FALSE;
    rendering_create_info.multisampling.alphaToOneEnable = VK_FALSE;
}

void VulkanOld::prepareRenderInfoDepthStencil() {
    rendering_create_info.depth_stencil = {};
    
    rendering_create_info.depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    rendering_create_info.depth_stencil.depthTestEnable = VK_TRUE;
    rendering_create_info.depth_stencil.depthWriteEnable = VK_TRUE;
    
    rendering_create_info.depth_stencil.depthBoundsTestEnable = VK_FALSE;
    rendering_create_info.depth_stencil.stencilTestEnable = VK_FALSE;
    
    rendering_create_info.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    
    rendering_create_info.depth_stencil.minDepthBounds = 0.0f;
    rendering_create_info.depth_stencil.maxDepthBounds = 1.0f;
    
    rendering_create_info.depth_stencil.front = {};
    rendering_create_info.depth_stencil.back = {};
}

void VulkanOld::prepareRenderInfoColorBlendAttachment() {
    rendering_create_info.color_blend_attachment = {};
    
    rendering_create_info.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    rendering_create_info.color_blend_attachment.blendEnable = VK_TRUE;
    
    rendering_create_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    rendering_create_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    rendering_create_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    
    rendering_create_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    rendering_create_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    rendering_create_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void VulkanOld::prepareRenderInfoColorBlendState() {
    rendering_create_info.color_blend_state = {};

    rendering_create_info.color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    rendering_create_info.color_blend_state.logicOpEnable = VK_FALSE;
    rendering_create_info.color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    
    rendering_create_info.color_blend_state.attachmentCount = 1;
    rendering_create_info.color_blend_state.pAttachments = &rendering_create_info.color_blend_attachment; //Change for all framebuffers
    
    rendering_create_info.color_blend_state.blendConstants[0] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[1] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[2] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[3] = 0.0f;
}

void VulkanOld::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &ubo_layout_binding;
    
    if (vkCreateDescriptorSetLayout(device, &create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS)
        log::error("Error creating the Vulkan Descriptor Set Layout for Uniform Buffers");
}

void VulkanOld::createPipelineLayout() {
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &descriptor_set_layout;
    
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        log::error("Error creating the Vulkan Pipeline Layout");
    
    log::graphics("Created the Vulkan Pipeline Layout");
}

void VulkanOld::createGraphicsPipeline() {
    std::vector<char> vert_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/test.vert.spv");
    std::vector<char> frag_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/test.frag.spv");
    
    Graphics::Shader::ShaderStages stages;
    stages.vert = Graphics::Shader::createShaderModule(vert_shader_code, device);
    stages.frag = Graphics::Shader::createShaderModule(frag_shader_code, device);
    std::vector<VkPipelineShaderStageCreateInfo> stage_info = Graphics::Shader::createShaderStageInfo(stages);
    
    createPipelineLayout();
    
    prepareRenderInfo();
    
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    create_info.stageCount = (int)stage_info.size();
    create_info.pStages = stage_info.data();
    
    create_info.pVertexInputState = &rendering_create_info.vertex_input;
    create_info.pInputAssemblyState = &rendering_create_info.input_assembly;
    create_info.pViewportState = &rendering_create_info.viewport_state;
    create_info.pRasterizationState = &rendering_create_info.rasterizer;
    create_info.pMultisampleState = &rendering_create_info.multisampling;
    create_info.pDepthStencilState = &rendering_create_info.depth_stencil;
    create_info.pColorBlendState = &rendering_create_info.color_blend_state;
    create_info.pDynamicState = nullptr;
    create_info.pTessellationState = nullptr;
    
    create_info.layout = pipeline_layout;
    
    create_info.renderPass = render_pass;
    create_info.subpass = 0;
    
    create_info.basePipelineHandle = VK_NULL_HANDLE; //It is not a derived pipeline
    create_info.basePipelineIndex = -1;
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        log::error("Error while creating the Vulkan Graphics Pipeline");
    
    log::graphics("Created the Vulkan Graphics Pipeline");
    
    vkDestroyShaderModule(device, stages.vert.value(), nullptr);
    vkDestroyShaderModule(device, stages.frag.value(), nullptr);
}

//----------------------------------------



//BUFFERS
//----------------------------------------

void VulkanOld::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory) {
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &create_info, nullptr, &buffer) != VK_SUCCESS)
        log::error("Failed to create a Vulkan Buffer");
    
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
    
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = getMemoryType(memory_requirements.memoryTypeBits, properties);
    
    //vkAllocate is discouraged for many components, see https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
    if (vkAllocateMemory(device, &allocate_info, nullptr, &memory) != VK_SUCCESS)
        log::error("Failed to allocate Buffer Memory");
    
    vkBindBufferMemory(device, buffer, memory, 0);
}

void VulkanOld::createVertexBuffer(const std::vector<Graphics::VertexData> &vertices) {
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    VkBufferUsageFlags staging_buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags staging_buffer_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    createBuffer(buffer_size, staging_buffer_usage, staging_buffer_properties, staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), (size_t) buffer_size);
    vkUnmapMemory(device, staging_buffer_memory);
    
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkMemoryPropertyFlags buffer_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createBuffer(buffer_size, buffer_usage, buffer_properties, vertex_buffer, vertex_buffer_memory);
    
    copyBuffer(staging_buffer, vertex_buffer, buffer_size);
    
    vkDestroyBuffer(device, staging_buffer, nullptr);
    vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void VulkanOld::createIndexBuffer(const std::vector<ui16> &indices) {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    index_buffer_size = (ui32)indices.size();
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    VkBufferUsageFlags staging_buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags staging_buffer_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    createBuffer(buffer_size, staging_buffer_usage, staging_buffer_properties, staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), (size_t) buffer_size);
    vkUnmapMemory(device, staging_buffer_memory);
    
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkMemoryPropertyFlags buffer_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createBuffer(buffer_size, buffer_usage, buffer_properties, index_buffer, index_buffer_memory);
    
    copyBuffer(staging_buffer, index_buffer, buffer_size);
    
    vkDestroyBuffer(device, staging_buffer, nullptr);
    vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void VulkanOld::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = temp_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);
    
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);
    
    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);
    
    vkFreeCommandBuffers(device, temp_command_pool, 1, &command_buffer);
}

ui32 VulkanOld::getMemoryType(ui32 filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    
    for (ui32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    
    log::error("Failed to find a suitable memory type");
    return 0;
}

//----------------------------------------



//RENDERING
//----------------------------------------

void VulkanOld::createFramebuffers() {
    swapchain_framebuffers.resize(swapchain_image_views.size());
    
    for (int i = 0; i < swapchain_framebuffers.size(); i++) {
        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &swapchain_image_views[i];
        create_info.width = swapchain_extent.width;
        create_info.height = swapchain_extent.height;
        create_info.layers = 1;
        
        if (vkCreateFramebuffer(device, &create_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Framebuffer");
    }
    
    log::graphics("Created all Vulkan Framebuffers");
}

void VulkanOld::createCommandPools() {
    //Main command pool
    
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_families.graphics_queue_family_index.value(); //This is the command pool for graphics
    create_info.flags = 0;
    
    if (vkCreateCommandPool(device, &create_info, nullptr, &command_pool) != VK_SUCCESS)
        log::error("Failed to create the Vulkan Graphics Command Pool");
    
    //Temporary command pool for short lived objects
    
    VkCommandPoolCreateInfo temp_create_info{};
    temp_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    temp_create_info.queueFamilyIndex = queue_families.graphics_queue_family_index.value();
    temp_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    
    if (vkCreateCommandPool(device, &create_info, nullptr, &temp_command_pool) != VK_SUCCESS)
        log::error("Failed to create the Vulkan Graphics Temporary Command Pool");
    
    log::graphics("Created all Vulkan Command Pools");
}

void VulkanOld::createCommandBuffers() {
    command_buffers.resize(swapchain_framebuffers.size());
    
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = (ui32)command_buffers.size();
    
    if (vkAllocateCommandBuffers(device, &allocate_info, command_buffers.data()) != VK_SUCCESS)
        log::error("Failed to allocate a Vulkan Command Buffer");
    
    for (int i = 0; i < command_buffers.size(); i++) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;
        
        if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Command Buffer");
        
        recordCommandBuffer(command_buffers[i], swapchain_framebuffers[i], descriptor_sets[i]);
        
        if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
            log::error("Failed to end recording on a Vulkan Command Buffer");
    }
    
    log::graphics("Created all Vulkan Command Buffers");
}

void VulkanOld::recordCommandBuffer(VkCommandBuffer &command_buffer, VkFramebuffer &framebuffer, VkDescriptorSet &descriptor_set) {
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.02f, 0.02f, 0.02f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};
    
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values.data();
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    VkBuffer vertex_buffers[]{ vertex_buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    
    vkCmdDrawIndexed(command_buffer, index_buffer_size, 1, 0, 0, 0);
    
    vkCmdEndRenderPass(command_buffer);
}

void VulkanOld::createSyncObjects() {
    semaphores_image_available.resize(MAX_FRAMES_IN_FLIGHT);
    semaphores_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
    fences_in_flight.resize(MAX_FRAMES_IN_FLIGHT);
    fences_images_in_flight.resize(swapchain_images.size(), VK_NULL_HANDLE);
    
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphores_image_available[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Semaphore");
        
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphores_render_finished[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Semaphore");
        
        if (vkCreateFence(device, &fence_info, nullptr, &fences_in_flight[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Fence");
    }
    
    log::graphics("Created all Vulkan Semaphores");
}

void VulkanOld::renderFrame(WindowData &win) {
    vkWaitForFences(device, 1, &fences_in_flight[current_frame], VK_TRUE, UINT64_MAX);
    
    VkResult result;
    ui32 image_index;
    result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores_image_available[current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(win);
        return;
    }
    if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR) {
        log::error("Failed to acquire Swapchain Image");
    }
    
    
    if (fences_images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &fences_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    fences_images_in_flight[image_index] = fences_in_flight[current_frame];
    
    VkSemaphore wait_semaphores[]{ semaphores_image_available[current_frame] };
    VkPipelineStageFlags wait_stages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[]{ semaphores_render_finished[current_frame] };
    
    
    updateUniformBuffer(image_index);
    
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[image_index];
    
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    
    vkResetFences(device, 1, &fences_in_flight[current_frame]);
    
    if (vkQueueSubmit(graphics_queue, 1, &submit_info, fences_in_flight[current_frame]) != VK_SUCCESS)
        log::error("Failed to submit Draw Command Buffer");
    
    
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[]{ swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    
    
    result = vkQueuePresentKHR(present_queue, &present_info);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
        recreateSwapchain(win);
    else if (result != VK_SUCCESS)
        log::error("Failed to present Swapchain Image");
    
    
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//----------------------------------------



//UNIFORM
//----------------------------------------

void VulkanOld::createUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(VK::UniformBufferObject);
    
    uniform_buffers.resize(swapchain_images.size());
    uniform_buffers_memory.resize(swapchain_images.size());
    
    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    for (int i = 0; i < swapchain_images.size(); i++)
        createBuffer(buffer_size, usage_flags, memory_flags, uniform_buffers[i], uniform_buffers_memory[i]);
    
    log::graphics("Created Vulkan Uniform Buffers");
}

void VulkanOld::createDescriptorPool() {
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = static_cast<ui32>(swapchain_images.size());
    
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;
    create_info.maxSets = static_cast<ui32>(swapchain_images.size());
    
    if (vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool) != VK_SUCCESS)
        log::error("Failed to create a Vulkan Descriptor Pool");
    
    log::graphics("Created a Vulkan Descriptor Pool");
}

void VulkanOld::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(swapchain_images.size(), descriptor_set_layout);
    
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = descriptor_pool;
    allocate_info.descriptorSetCount = static_cast<ui32>(swapchain_images.size());
    allocate_info.pSetLayouts = layouts.data();
    
    descriptor_sets.resize(swapchain_images.size());
    if (vkAllocateDescriptorSets(device, &allocate_info, descriptor_sets.data()) != VK_SUCCESS)
        log::error("Failed to allocate Vulkan Descriptor Sets");
    
    log::graphics("Allocated Vulkan Descriptor Sets");
    
    for (int i = 0; i < swapchain_images.size(); i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE; //Also possible to use sizeof(VK::UniformBufferObject);
        
        VkWriteDescriptorSet write_descriptor{};
        write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor.dstSet = descriptor_sets[i];
        write_descriptor.dstBinding = 0; //Binding, specified in the shader
        write_descriptor.dstArrayElement = 0;
        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor.descriptorCount = 1;
        write_descriptor.pBufferInfo = &buffer_info;
        write_descriptor.pImageInfo = nullptr;
        write_descriptor.pTexelBufferView = nullptr;
        
        vkUpdateDescriptorSets(device, 1, &write_descriptor, 0, nullptr);
    }
}

void VulkanOld::updateUniformBuffer(ui32 current_image) {
    //EXAMPLE FUNCTION
    
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    VK::UniformBufferObject ubo{};
    
    ubo.model = glm::rotate(glm::mat4(1.0f), t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, 0.3f * std::sin(t * 1.570796f)));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float) swapchain_extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    
    void* data;
    vkMapMemory(device, uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniform_buffers_memory[current_image]);
}

//----------------------------------------



//TEST
//----------------------------------------

void API::renderTest(WindowData &win, RenderData &render) {
    render.api.renderFrame(win);
}

//----------------------------------------



//CLEANUP
//----------------------------------------

void VulkanOld::cleanSwapchain() {
    for (VkFramebuffer fb : swapchain_framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);
    
    vkFreeCommandBuffers(device, command_pool, static_cast<ui32>(command_buffers.size()), command_buffers.data());
    
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    
    for (VkImageView view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);
    
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    
    for (int i = 0; i < swapchain_images.size(); i++) {
        vkDestroyBuffer(device, uniform_buffers[i], nullptr);
        vkFreeMemory(device, uniform_buffers_memory[i], nullptr);
    }
    
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
}

void VulkanOld::clean() {
    vkDeviceWaitIdle(device);
    
    cleanSwapchain();
    
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    
    vkDestroyBuffer(device, vertex_buffer, nullptr);
    vkFreeMemory(device, vertex_buffer_memory, nullptr);
    
    vkDestroyBuffer(device, index_buffer, nullptr);
    vkFreeMemory(device, index_buffer_memory, nullptr);
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, semaphores_image_available[i], nullptr);
        vkDestroySemaphore(device, semaphores_render_finished[i], nullptr);
        vkDestroyFence(device, fences_in_flight[i], nullptr);
    }
    
    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroyCommandPool(device, temp_command_pool, nullptr);
    
    vkDestroyDevice(device, nullptr);
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    log::graphics("Cleaned up Vulkan");
}

void API::clean(RenderData &render) {
    render.api.clean();
}

//----------------------------------------

#endif
