//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

#include <map>
#include <set>

#include "r_shader.h"

using namespace Verse;
using namespace Graphics;

namespace Verse::Graphics
{

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> required_device_extensions = {
    "VK_KHR_portability_subset",
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//DEVICE
//----------------------------------------

void Vulkan::createInstance(Config &c) {
    log::graphics("");
    
    ui32 extension_count;
    SDL_Vulkan_GetInstanceExtensions(c.window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(c.window, &extension_count, extension_names.data());
    log::graphics("Vulkan extensions needed: %d", extension_count);
    for (const char* ext : extension_names)
        log::graphics(" - %s", ext);
    
    log::graphics("");
    
    ui32 validation_layer_count;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
    log::graphics("Vulkan validation layers supported: %d", validation_layer_count);
    for (const auto &layer : available_validation_layers)
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
    
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = c.name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(c.version[0], c.version[1], c.version[2]);
    app_info.pEngineName = "Fresa";
    app_info.engineVersion = VK_MAKE_VERSION(c.version[0], c.version[1], c.version[2]);
    app_info.apiVersion = VK_API_VERSION_1_1;
    
    VkInstanceCreateInfo instance_create_info = {};
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
    
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    log::graphics("Vulkan physical devices: %d", device_count);
    
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
    
    std::set<ui32> unique_queue_families = {};
    if (queue_families.graphics_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.graphics_queue_family_index.value());
    if (queue_families.present_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.present_queue_family_index.value());
    if (queue_families.compute_queue_family_index.has_value())
        unique_queue_families.insert(queue_families.compute_queue_family_index.value());
    
    int i = 0;
    std::vector<float> priorities = { 1.0f, 1.0f, 0.5f };
    
    for (ui32 family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_graphics_info = {};
        queue_graphics_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_graphics_info.queueFamilyIndex = family;
        queue_graphics_info.queueCount = 1;
        queue_graphics_info.pQueuePriorities = &priorities[i];
        queue_create_infos.push_back(queue_graphics_info);
        i++;
    }
    
    //Device required features
    VkPhysicalDeviceFeatures device_features = {};
    
    VkDeviceCreateInfo device_create_info = {};
    
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
    
    if (queue_families.graphics_queue_family_index.has_value())
        vkGetDeviceQueue(device, queue_families.graphics_queue_family_index.value(), 0, &graphics_queue);
    if (queue_families.compute_queue_family_index.has_value())
        vkGetDeviceQueue(device, queue_families.compute_queue_family_index.value(), 0, &compute_queue);
    if (queue_families.present_queue_family_index.has_value())
        vkGetDeviceQueue(device, queue_families.present_queue_family_index.value(), 0, &present_queue);
}

void Vulkan::createSurface(Config &c) {
    if (not SDL_Vulkan_CreateSurface(c.window, instance, &surface))
        log::error("Error while creating a Vulkan Surface (from createSurface): %s", SDL_GetError());
}

//----------------------------------------



//SWAPCHAIN
//----------------------------------------

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
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::selectSwapExtent(Config &c, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(c.window, &w, &h);
    
    VkExtent2D actual_extent = { static_cast<ui32>(w), static_cast<ui32>(h) };
    
    std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actual_extent;
}

void Vulkan::createSwapchain(Config &c) {
    VK::SwapchainSupportDetails swapchain_support = getSwapchainSupport(physical_device);
    
    VkSurfaceFormatKHR surface_format = selectSwapSurfaceFormat(swapchain_support.formats);
    VkPresentModeKHR present_mode = selectSwapPresentMode(swapchain_support.present_modes);
    swapchain_extent = selectSwapExtent(c, swapchain_support.capabilities);
    swapchain_format = surface_format.format;
    
    ui32 image_count = swapchain_support.capabilities.minImageCount + 1;
    
    if (swapchain_support.capabilities.maxImageCount > 0 and image_count > swapchain_support.capabilities.maxImageCount)
        image_count = swapchain_support.capabilities.maxImageCount;
    
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swapchain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    ui32 queue_family_indices[2] = {
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
        log::error("Error creating Vulkan Swapchain");
    
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());
}

void Vulkan::createImageViews() {
    swapchain_image_views.resize(swapchain_images.size());
    
    for (int i = 0; i < swapchain_images.size(); i++)
        swapchain_image_views[i] = createImageView(swapchain_images[i], VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView Vulkan::createImageView(VkImage image, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo create_info = {};
    
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
        log::error("Error creating Vulkan Image View");
    
    return image_view;
}

//----------------------------------------



//PIPELINE
//----------------------------------------

void Vulkan::createRenderPass() {
    VkAttachmentDescription color_attachment = {};
    
    color_attachment.format = swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkSubpassDescription subpass = createRenderSubpass();
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    
    if (vkCreateRenderPass(device, &create_info, nullptr, &render_pass) != VK_SUCCESS)
        log::error("Error creating a Vulkan Render Pass");
}

VkSubpassDescription Vulkan::createRenderSubpass() {
    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    
    return subpass;
}

void Vulkan::prepareRenderInfo() {
    prepareRenderInfoVertexInput();
    prepareRenderInfoInputAssembly();
    prepareRenderInfoViewportState();
    prepareRenderInfoRasterizer();
    prepareRenderInfoMultisampling();
    prepareRenderInfoDepthStencil();
    prepareRenderInfoColorBlendAttachment();
    prepareRenderInfoColorBlendState();
}

void Vulkan::prepareRenderInfoVertexInput() {
    rendering_create_info.vertex_input = {};
    
    rendering_create_info.vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    rendering_create_info.vertex_input.vertexBindingDescriptionCount = 0;
    rendering_create_info.vertex_input.pVertexBindingDescriptions = nullptr;
    rendering_create_info.vertex_input.vertexAttributeDescriptionCount = 0;
    rendering_create_info.vertex_input.pVertexAttributeDescriptions = nullptr;
}

void Vulkan::prepareRenderInfoInputAssembly() {
    rendering_create_info.input_assembly = {};
    
    rendering_create_info.input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    rendering_create_info.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    rendering_create_info.input_assembly.primitiveRestartEnable = VK_FALSE; //Change when using an index buffer
}

void Vulkan::prepareRenderInfoViewportState() {
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

void Vulkan::prepareRenderInfoRasterizer() {
    rendering_create_info.rasterizer = {};
    
    rendering_create_info.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rendering_create_info.rasterizer.depthClampEnable = VK_FALSE;
    rendering_create_info.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    
    rendering_create_info.rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Fill, Line, Point, requires enabling GPU features
    rendering_create_info.rasterizer.lineWidth = 1.0f; //Larger thickness requires enabling GPU features
    
    rendering_create_info.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rendering_create_info.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    
    rendering_create_info.rasterizer.depthBiasEnable = VK_FALSE;
    rendering_create_info.rasterizer.depthBiasConstantFactor = 0.0f;
    rendering_create_info.rasterizer.depthBiasClamp = 0.0f;
    rendering_create_info.rasterizer.depthBiasSlopeFactor = 0.0f;
}

void Vulkan::prepareRenderInfoMultisampling() {
    rendering_create_info.multisampling = {};
    
    rendering_create_info.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    rendering_create_info.multisampling.sampleShadingEnable = VK_FALSE; //Needs to be enabled in the future
    
    rendering_create_info.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    rendering_create_info.multisampling.minSampleShading = 1.0f;
    rendering_create_info.multisampling.pSampleMask = nullptr;
    
    rendering_create_info.multisampling.alphaToCoverageEnable = VK_FALSE;
    rendering_create_info.multisampling.alphaToOneEnable = VK_FALSE;
}

void Vulkan::prepareRenderInfoDepthStencil() {
    rendering_create_info.depth_stencil = {};
    
    //TODO: Enable this
}

void Vulkan::prepareRenderInfoColorBlendAttachment() {
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

void Vulkan::prepareRenderInfoColorBlendState() {
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

void Vulkan::createPipelineLayout() {
    VkPipelineLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = nullptr;
    
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        log::error("Error creating a Vulkan Pipeline Layout");
}

void Vulkan::createGraphicsPipeline() {
    std::vector<char> vert_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/vert.spv");
    std::vector<char> frag_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/frag.spv");
    
    Graphics::Shader::ShaderStages stages;
    stages.vert = Graphics::Shader::createShaderModule(vert_shader_code, device);
    stages.frag = Graphics::Shader::createShaderModule(frag_shader_code, device);
    std::vector<VkPipelineShaderStageCreateInfo> stage_info = Graphics::Shader::createShaderStageInfo(stages);
    
    createPipelineLayout();
    
    prepareRenderInfo();
    
    VkGraphicsPipelineCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    create_info.stageCount = (int)stage_info.size();
    create_info.pStages = stage_info.data();
    
    create_info.pVertexInputState = &rendering_create_info.vertex_input;
    create_info.pInputAssemblyState = &rendering_create_info.input_assembly;
    create_info.pViewportState = &rendering_create_info.viewport_state;
    create_info.pRasterizationState = &rendering_create_info.rasterizer;
    create_info.pMultisampleState = &rendering_create_info.multisampling;
    create_info.pDepthStencilState = nullptr; //Add this
    create_info.pColorBlendState = &rendering_create_info.color_blend_state;
    create_info.pDynamicState = nullptr; //And this
    create_info.pTessellationState = nullptr;
    
    create_info.layout = pipeline_layout;
    
    create_info.renderPass = render_pass;
    create_info.subpass = 0;
    
    create_info.basePipelineHandle = VK_NULL_HANDLE; //It is not a derived pipeline
    create_info.basePipelineIndex = -1;
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        log::error("Error while creating a Vulkan Graphics Pipeline");
    
    vkDestroyShaderModule(device, stages.vert.value(), nullptr);
    vkDestroyShaderModule(device, stages.frag.value(), nullptr);
}

//----------------------------------------



//CLEANUP
//----------------------------------------

void Vulkan::destroy() {
    vkDestroyRenderPass(device, render_pass, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    
    for (VkImageView view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);
    
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

//----------------------------------------
}

#endif
