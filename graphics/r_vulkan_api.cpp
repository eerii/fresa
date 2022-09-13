//* vulkan_api
//      this file includes the vulkan specific functions that extend the main api

#include "r_api.h"
#include "log.h"
#include "string_utils.h"
#include "fresa_assert.h"
#include "engine.h"

#include <set>

#define VMA_IMPLEMENTATION
#ifndef _MSC_VER
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Weverything"
    #include "vk_mem_alloc.h"
    #pragma clang diagnostic pop
#else
    #include "vk_mem_alloc.h"
#endif

using namespace fresa;
using namespace graphics;

// ·····························
// · VULKAN SPECIFIC FUNCTIONS ·
// ·····························

namespace fresa::graphics::vk
{
    //: instance
    VkInstance createInstance(DeletionQueue& dq);

    //: physical device
    vk::GPU selectGPU(VkInstance instance);
    int rateGPU(VkInstance instance, const vk::GPU &gpu);
    decltype(vk::GPU{}.queue_indices) getQueueIndices(VkInstance instance, const vk::GPU &gpu);

    //: logical device
    VkDevice createDevice(const vk::GPU &gpu, DeletionQueue& dq);
    decltype(vk::GPU{}.queues) getQueues(const vk::GPU &gpu);

    //: allocator
    VmaAllocator createAllocator(VkInstance instance, const vk::GPU &gpu, DeletionQueue& dq);

    //: surface and swapchain
    VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window, DeletionQueue& dq);
    vk::Swapchain createSwapchain(const vk::GPU &gpu, GLFWwindow* window, VkSurfaceKHR surface, DeletionQueue& dq);

    //: command pools
    void initFrameCommands(decltype(api->frame) &frame, const vk::GPU &gpu, DeletionQueue& dq);
    
    //: debug
    void glfwErrorCallback(int error, const char* description);
    #ifdef FRESA_DEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, ui64 obj, std::size_t location, int code, const char* layer_prefix, const char* msg, void* user_data);
    VkDebugReportCallbackEXT createDebugCallback(VkInstance instance, DeletionQueue& dq);
    #endif
}

// ·····························
// · VULKAN SPECIFIC VARIABLES ·
// ·····························

namespace fresa::graphics::vk
{
    //: required device extensions
    constexpr std::array<const char*, 2> required_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    //: required validation layers
    constexpr std::array<const char*, 1> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
}

// ··················
// · INITIALIZATION ·
// ··················

//* create vulkan graphics api
//      extends the graphics system initialization function by calling all the vulkan specific creation code
void GraphicsSystem::init() {
    //: initalize glfw
    glfwSetErrorCallback(vk::glfwErrorCallback);
    graphics_assert(glfwInit(), "failed to initalize glfw");

    //: check for vulkan support
	graphics_assert(glfwVulkanSupported(), "a vulkan loader has not been found");
    int version = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
    graphics_assert(version, "glad failed to load vulkan");
    log::graphics("glad vulkan loader ({}.{})", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    //: create window
    win = std::make_unique<const Window>(createWindow());

    //: stop the engine on window close
    glfwSetWindowCloseCallback(win->window, [](GLFWwindow* window) { fresa::quit(); });

    //: create vulkan api
    VulkanAPI vk_api;

    //: instance
    vk_api.instance = vk::createInstance(vk_api.deletion_queue_global);
    version = gladLoaderLoadVulkan(vk_api.instance, nullptr, nullptr);
    graphics_assert(version, "glad failed to load the vulkan functions that require an instance");
    #ifdef FRESA_DEBUG
    vk_api.debug_callback = vk::createDebugCallback(vk_api.instance, vk_api.deletion_queue_global);
    #endif

    //: gpu (physical device)
    vk_api.gpu = vk::selectGPU(vk_api.instance);
    version = gladLoaderLoadVulkan(vk_api.instance, vk_api.gpu.gpu, nullptr);
    graphics_assert(version, "glad failed to load the extra vulkan extensions required by the gpu");

    //: logical device
    vk_api.gpu.device = vk::createDevice(vk_api.gpu, vk_api.deletion_queue_global);
    vk_api.gpu.queues = vk::getQueues(vk_api.gpu);

    //: allocator
    vk_api.allocator = vk::createAllocator(vk_api.instance, vk_api.gpu, vk_api.deletion_queue_global);

    //: surface and swapchain
    vk_api.surface = vk::createSurface(vk_api.instance, win->window, vk_api.deletion_queue_global);
    vk_api.swapchain = vk::createSwapchain(vk_api.gpu, win->window, vk_api.surface, vk_api.deletion_queue_swapchain);

    //: command pools
    initFrameCommands(vk_api.frame, vk_api.gpu, vk_api.deletion_queue_global);

    //: save the api pointer, can't be modified after this point
    api = std::make_unique<const VulkanAPI>(std::move(vk_api));
}

//* create vulkan instance
//      an instance is a handle to the vulkan library, required in almost every vulkan function
//      it includes details on how the application connects to the driver, as well as validation layers and some extensions
VkInstance vk::createInstance(DeletionQueue& dq) {
    //: instance extensions
    //      platform specific extensions needed to create the window surface and other utils, such as the debug messenger
    ui32 instance_extension_count = 0;
    const char** instance_extension_buffer = glfwGetRequiredInstanceExtensions(&instance_extension_count);
    std::vector<const char*> instance_extensions(instance_extension_count);
    log::graphics("instance extensions:");
    for (ui32 i = 0; i < instance_extension_count; i++)
        instance_extensions.at(i) = instance_extension_buffer[i];
    instance_extensions.push_back("VK_KHR_portability_enumeration");
    #ifdef FRESA_DEBUG
    instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    #endif
    for (auto ext : instance_extensions)
        log::graphics(" - {}", lower(ext));

    //: validation layers
    //      middleware for existing vulkan functionality
    //      primarily used for getting detailed error descriptions, in this case with VK_LAYER_KHRONOS_validation
    //      enabled only when using debug mode
    #ifdef FRESA_DEBUG
    ui32 validation_layer_count;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());
    log::graphics("validation layers:");
    for (auto layer : validation_layers) {
        auto it = std::find_if(available_validation_layers.begin(), available_validation_layers.end(), [&](const auto& l) { return l.layerName == str_view(layer); });
        if (it == available_validation_layers.end()) log::warn("requested validation layer '{}' not found", lower(layer));
        log::graphics(" - {}", lower(layer));
    }
    #endif

    //: application info
    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = engine_config.name().data(),
        .applicationVersion = VK_MAKE_VERSION(engine_config.version().at(0), engine_config.version().at(1), engine_config.version().at(2)),
        .pEngineName = "fresa",
        .engineVersion = VK_MAKE_VERSION(EngineConfig{}.version().at(0), EngineConfig{}.version().at(1), EngineConfig{}.version().at(2)),
        .apiVersion = VK_API_VERSION_1_1
    };
    
    //: instance info
    VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &app_info
    };
    create_info.enabledExtensionCount = (int)instance_extensions.size();
    create_info.ppEnabledExtensionNames = instance_extensions.data();
    #ifdef FRESA_DEBUG
    create_info.enabledLayerCount = (int)validation_layers.size();
    create_info.ppEnabledLayerNames = validation_layers.data();
    #endif

    //: create instance
    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    graphics_assert<int>(result == VK_SUCCESS, "failed to create a vulkan instance: {}", result);

    dq.push([instance]{ vkDestroyInstance(instance, nullptr); });
    log::graphics("created a vulkan instance");
    return instance;
}

//* select physical device
//      a physical device is a representation of one of the gpus in the system
//      it can be used to query details on the gpu's capabilities and properties
//      this function returns a vk::GPU object, that includes a VkPhysicalDevice, but also its properties, features and memory information
//      it will also include a list of queue family indices, which can then be used to create the command queues that the program requires
vk::GPU vk::selectGPU(VkInstance instance) {
    //: count devices
    ui32 gpu_count;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    graphics_assert<int>(result == VK_SUCCESS, "fatal error enumerating physical devices: {}", result);
    graphics_assert(gpu_count > 0, "no physical devices found");
    
    //: get the available gpus
    std::vector<VkPhysicalDevice> physical_devices(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data());

    //: fill the gpu list with all the relevant information
    std::vector<vk::GPU> gpus;
    for (auto d : physical_devices) {
        gpus.push_back(vk::GPU{ .gpu = d });
        vk::GPU &gpu = gpus.back();

        //: get properties
        vkGetPhysicalDeviceProperties(gpu.gpu, &gpu.properties);
        vkGetPhysicalDeviceFeatures(gpu.gpu, &gpu.features);
        vkGetPhysicalDeviceMemoryProperties(gpu.gpu, &gpu.memory);
        gpu.queue_indices = vk::getQueueIndices(instance, gpu);

        //: rate the gpu
        gpu.score = rateGPU(instance, gpu);
    }

    //: get the gpu with the highest score
    auto it = std::max_element(gpus.begin(), gpus.end(), [](const auto& a, const auto& b) { return a.score < b.score; });
    graphics_assert(it != gpus.end() and it->score > 0, "no suitable gpu found");

    //: log the chosen gpu and the required device extensions
    log::graphics("gpu{}:", gpu_count > 1 ? "s" : "");
    for (auto &gpu : gpus)
        log::graphics(" {} {}", gpu.properties.deviceID == it->properties.deviceID ? "-" : "·", lower(gpu.properties.deviceName));
    log::graphics("required extensions:");
    for (auto &ext : required_extensions)
        log::graphics(" - {}", lower(ext));

    return *it;
}

//* rate physical device
//      give a score to the gpu based on its capabilities
//      if the score returned is 0, the gpu is invalid, otherwise selectGPU will choose the one with the highest score
int vk::rateGPU(VkInstance instance, const vk::GPU &gpu) {
    //: assertions to ensure the gpu is in a valid state
    fresa_assert(gpu.gpu != VK_NULL_HANDLE, "the gpu object has not been initialized");
    fresa_assert(gpu.score == -1, "the gpu has already been rated");

    int score = 16;

    //: prefer discrete gpus
    if (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 256;
    else if (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 16;

    //: prefer higher memory (only for discrete gpus)
    auto heaps = std::vector<VkMemoryHeap>(gpu.memory.memoryHeaps, gpu.memory.memoryHeaps + gpu.memory.memoryHeapCount);
    for (const auto& heap : heaps)
        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT and gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += heap.size / 1024 / 1024 / 128;

    //: required device extensions
    ui32 extension_count;
    vkEnumerateDeviceExtensionProperties(gpu.gpu, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(gpu.gpu, nullptr, &extension_count, available_extensions.data());
    for (const auto& ext : required_extensions) {
        auto it = std::find_if(available_extensions.begin(), available_extensions.end(), [&](const auto& e) { return e.extensionName == str_view(ext); });
        if (it == available_extensions.end()) { return 0; }
    }

    //: require present and graphics queues, compute adds extra score
    if (gpu.queue_indices.at(QUEUE_INDICES_PRESENT) == -1 or gpu.queue_indices[QUEUE_INDICES_GRAPHICS] == -1) return 0;
    if (gpu.queue_indices.at(QUEUE_INDICES_COMPUTE) != -1) score += 32;

    //- check swapchain support

    return score;
}

//* get queue indices
//      find the indices of the suitable queues that the program requires
//      it will return an ordered array of indices, indexed by the enum QUEUE_INDICES
//      the different types we are looking for are:
//          + graphics: pipeline operations, including vertex/fragment shaders and drawing
//          + present: send framebuffers to the screen
//          + transfer: copy buffers and images
//          + compute: for compute shaders
//      not all queues are needed, and more can be created for multithread support
//      the present and graphics queue can share the same index
decltype(vk::GPU{}.queue_indices) vk::getQueueIndices(VkInstance instance, const vk::GPU &gpu) {
    //: assertions to ensure the gpu is in a valid state
    fresa_assert(gpu.gpu != VK_NULL_HANDLE, "the gpu object has not been initialized");
    fresa_assert(gpu.queue_indices.at(QUEUE_INDICES_GRAPHICS) == -1, "the queue indices for this gpu have already been found");

    //: create the index array
    decltype(vk::GPU{}.queue_indices) indices;
    for (auto &i : indices) i = -1;
    
    //: get queue families
    ui32 queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.gpu, &queue_count, queues.data());

    //: select the desired queues
    for (int i = 0; i < queue_count; i++) {
        //: present queue
        if (indices.at(QUEUE_INDICES_PRESENT) == -1) {
            if (glfwGetPhysicalDevicePresentationSupport(instance, gpu.gpu, i))
                indices.at(QUEUE_INDICES_PRESENT) = i;
        }
        //: graphics queue (can be the same index)
        if (indices.at(QUEUE_INDICES_GRAPHICS) == -1 and queues.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.at(QUEUE_INDICES_GRAPHICS) = i; continue;
        }
        //: transfer queue
        if (indices.at(QUEUE_INDICES_TRANSFER) == -1 and queues.at(i).queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.at(QUEUE_INDICES_TRANSFER) = i; continue;
        }
        //: compute queue
        if (indices.at(QUEUE_INDICES_COMPUTE) == -1 and queues.at(i).queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.at(QUEUE_INDICES_COMPUTE) = i; continue;
        }
        if (indices.at(QUEUE_INDICES_GRAPHICS) != -1 and indices.at(QUEUE_INDICES_PRESENT) != -1 and
            indices.at(QUEUE_INDICES_TRANSFER) != -1 and indices.at(QUEUE_INDICES_COMPUTE) != -1) break;
    }

    return indices;
}

//* create logical device
//      this is the actual gpu driver for the selected gpu, and the main way to communicate with it
//      like an instance, this handle will be used throughout the program in most vulkan functions
//      when creating the logical device, a list of extensions can be passed to enable certain features
VkDevice vk::createDevice(const vk::GPU &gpu, DeletionQueue& dq) {
    //: assertions to ensure the gpu is in a valid state
    fresa_assert(gpu.gpu != VK_NULL_HANDLE, "the gpu object has not been initialized");
    fresa_assert(gpu.queue_indices.at(QUEUE_INDICES_GRAPHICS) != -1, "the gpu queue indices have not been initialized");
    fresa_assert(gpu.device == VK_NULL_HANDLE, "the gpu device has already been initialized");

    //: get unique queue indices
    std::map<int, float> unique_queue_indices;
    constexpr std::size_t queue_count = decltype(vk::GPU::queue_indices){}.size();
    constexpr std::array<float, queue_count> queue_priorities = { 1.0f, 1.0f, 0.5f, 0.5f};
    for (const auto& index : gpu.queue_indices) {
        if (not unique_queue_indices.contains(index))
            unique_queue_indices[index] = queue_priorities.at(index);
    }
    
    //: queue create info
    std::vector<VkDeviceQueueCreateInfo> queue_create_info(unique_queue_indices.size());
    for (const auto& [family_index, priority] : unique_queue_indices) {
        auto &info = queue_create_info.at(family_index);
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family_index;
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
    }

    //: device required features
    //      you may enable some features here, but try to keep them at the minimum
    VkPhysicalDeviceFeatures enabled_device_features{};

    //: device create info
    VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .enabledLayerCount = (int)validation_layers.size(),
        .ppEnabledLayerNames = validation_layers.data(),
        .enabledExtensionCount = (int)required_extensions.size(),
        .ppEnabledExtensionNames = required_extensions.data(),
        .pEnabledFeatures = &enabled_device_features,
    };
    create_info.queueCreateInfoCount = (int)queue_create_info.size();
    create_info.pQueueCreateInfos = queue_create_info.data();

    //: create logical device
    VkDevice device;
    VkResult result = vkCreateDevice(gpu.gpu, &create_info, nullptr, &device);
    graphics_assert<int>(result == VK_SUCCESS, "failed to create a vulkan logical device: {}", result);

    dq.push([device]{ vkDeviceWaitIdle(device); vkDestroyDevice(device, nullptr); });
    log::graphics("created a vulkan gpu device");
    return device;
}

//: get queue handles
//      use the logical device information to retrieve the actual VkQueue handles
//      the queue handles are used to submit commands to the gpu
decltype(vk::GPU{}.queues) vk::getQueues(const vk::GPU &gpu) {
    //: assertions to ensure the gpu is in a valid state
    fresa_assert(gpu.device != VK_NULL_HANDLE, "the gpu logical device has not been initialized");
    fresa_assert(gpu.queues.size() == 0, "the gpu queue handles have already been initialized");
    
    //: get unique queue indices
    std::set<int> unique_queue_indices;
    for (const auto& index : gpu.queue_indices)
        unique_queue_indices.insert(index);

    //: get queue handles
    decltype(vk::GPU{}.queues) handles(unique_queue_indices.size());
    for (const auto& index : gpu.queue_indices)
        vkGetDeviceQueue(gpu.device, index, 0, &handles.at(index));

    return handles;
}

//* create memory allocator
//      memory management in vulkan is really tedious, since there are many memory types (cpu, gpu...) with different limitations and speeds
//      buffers and images have to be accompained by a VkDeviceMemory which needs to be allocated by vkAllocateMemory
//      the problem is that it is discouraged to call vkAllocateMemory per buffer, since the number of allowed allocations is small even on top tier hardware, for example, 4096 on a gtx 1080
//      the solution is to allocate big chunks of memory and then suballocate to each resource, using offsets and keeping track of where each buffer resides and how big it is
//      this is hard to do right while avoiding fragmentation and overlaps, so we are integrating the vulkan memory allocator library
//      it takes care of the big chunk allocation and position for us.
//      it is possible to write a smaller tool to help, but in an attempt to keep the scope of the project manageable (says the one writing vulkan for a 2d tiny engine...) we'll leave it for now
//      here we create the VmaAllocator object, which we will have to reference during buffer creation and will house the pools of memory that we will be using
VmaAllocator vk::createAllocator(VkInstance instance, const vk::GPU &gpu, DeletionQueue& dq) {
    //: dinamically load vma functions
    VmaVulkanFunctions vma_functions = {};
    vma_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    //: create info
    VmaAllocatorCreateInfo create_info{
        .physicalDevice = gpu.gpu,
        .device = gpu.device,
        .pVulkanFunctions = &vma_functions,
        .instance = instance,
    };

    //: create allocator
    VmaAllocator allocator;
    VkResult result = vmaCreateAllocator(&create_info, &allocator);
    graphics_assert<int>(result == VK_SUCCESS, "failed to create a vulkan vma allocator: {}", result);

    dq.push([allocator]{ vmaDestroyAllocator(allocator); });
    log::graphics("created a vma allocator");
    return allocator;
}

//* create a surface
//      a surface is an abstraction of the window that vulkan can render to
VkSurfaceKHR vk::createSurface(VkInstance instance, GLFWwindow* window, DeletionQueue& dq) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    graphics_assert<int>(result == VK_SUCCESS, "failed to create a vulkan surface: {}", result);

    dq.push([instance, surface]{ vkDestroySurfaceKHR(instance, surface, nullptr); });
    return surface;
}

//* create a swapchain
//      a swapchain is a collection of images that are used to render to the screen
//      they are not core in the vulkan specification since they are optional (for example, while running vulkan headless) and different on every platform
//      the swapchain has a determined extent, so it (and everything that depends on it) must be recreated when the window is resized
//      there are also different presentation modes available (no vsync, vsync, mailbox...)
vk::Swapchain vk::createSwapchain(const vk::GPU &gpu, GLFWwindow* window, VkSurfaceKHR surface, DeletionQueue& dq) {
    //: select format
    //      this is the internal format for the vulkan surface, vulkan automatically converts our framebuffers to this space so we don't need to worry
    ui32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.gpu, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.gpu, surface, &format_count, formats.data());

    VkSurfaceFormatKHR format = (formats.size() == 1 && formats.at(0).format == VK_FORMAT_UNDEFINED) ? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} : formats.at(0);

    //: select present mode
    //      this indicates the way vulkan will present the images to the screen
    //      + immediate: no vsync, the images are presented directly to the screen
    //      + fifo: vsync, when the queue is full the program waits
    //      + mailbox: triple buffering, the program replaces the last images of the queue, less latency but more power consumption
    //      not all gpus support mailbox (for example integrated intel gpus), so while it is preferred, fifo can be used as a fallback
    ui32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.gpu, surface, &present_mode_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.gpu, surface, &present_mode_count, present_modes.data());

    VkPresentModeKHR present_mode = [&]{
        //: check if mailbox is preferred and available, if not use fifo
        if (config.vk_prefer_mailbox_mode)
            for (const auto& mode : present_modes)
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return mode;
        return VK_PRESENT_MODE_FIFO_KHR;
    }();

    //: surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.gpu, surface, &capabilities);

    //: select extent
    //      this is the drawable area on the screen
    //      if the current extent is uint32_max, we should calculate the actual extent using Window and clamp it to the min and max supported extent by the gpu
    VkExtent2D extent = [&]{
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        VkExtent2D actual_extent{ (ui32)w, (ui32)h };
        std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }();

    //: minimum number of images in the swapchain
    //      we should ask for at least the minimum number of images in the swapchain, and one more as a good practise to avoid blocking
    //      we may get more images than the minimum, for example, when using mailbox mode, so no code should be based on this number
    //      there is also a check to see that we are not requesting more images than the gpu supports
    ui32 min_image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && min_image_count > capabilities.maxImageCount)
        min_image_count = capabilities.maxImageCount;

    //: create info
    VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,

        .minImageCount = min_image_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    //: specify the sharing mode for the queues
    std::array<ui32, 2> queue_family_indices;
    queue_family_indices.at(0) = (int)gpu.queue_indices.at(QUEUE_INDICES_GRAPHICS);
    queue_family_indices.at(1) = (int)gpu.queue_indices.at(QUEUE_INDICES_PRESENT);
    if (queue_family_indices.at(0) != queue_family_indices.at(1)) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    //: create swapchain
    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(gpu.device, &create_info, nullptr, &swapchain);
    graphics_assert(result == VK_SUCCESS, "failed to create a vulkan swapchain");

    //: get swapchain images
    ui32 image_count;
    vkGetSwapchainImagesKHR(gpu.device, swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(gpu.device, swapchain, &image_count, images.data());

    //: create image views
    std::vector<VkImageView> image_views(images.size());
    for (ui32 i = 0; i < images.size(); i++) {
        //- move to vk::createImageView()
        VkImageViewCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format.format,
            .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        VkResult result = vkCreateImageView(gpu.device, &create_info, nullptr, &image_views.at(i));
        graphics_assert(result == VK_SUCCESS, "failed to create a vulkan image view for the swapchain");
    }

    //: create swapchain object
    Swapchain swapchain_data{
        .swapchain = swapchain,
        .format = format.format,
        .extent = extent,
        .image_views = image_views,
        .images = images
    };
    swapchain_data.size = (ui32)images.size();

    //: deletion queue
    dq.push([swapchain_data, device = gpu.device]{
        for (VkImageView view : swapchain_data.image_views)
            vkDestroyImageView(device, view, nullptr);
        vkDestroySwapchainKHR(device, swapchain_data.swapchain, nullptr);
    });

    log::graphics("created a swapchain: {} images, ({} {}) extent, {} present mode",
                  images.size(), extent.width, extent.height, present_mode == VK_PRESENT_MODE_MAILBOX_KHR ? "mailbox" : "fifo");
    return swapchain_data;
}

//* initialize render commands
void vk::initFrameCommands(decltype(api->frame) &frame, const vk::GPU &gpu, DeletionQueue& dq) {
    //: create info for graphics command pools
    VkCommandPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = (ui32)gpu.queue_indices.at(QUEUE_INDICES_GRAPHICS)
    };

    //: allocate info for command buffers (without the command pool)
    constexpr VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    for_<0, engine_config.vk_frames_in_flight()>([&](auto i){
        //: checks for valid state
        fresa_assert(frame.at(i).command_pool == VK_NULL_HANDLE, "command pool already initialized");
        fresa_assert(frame.at(i).main_command_buffer == VK_NULL_HANDLE, "command buffer already initialized");

        //: create command pool
        VkResult result = vkCreateCommandPool(gpu.device, &create_info, nullptr, &frame.at(i).command_pool);
        graphics_assert<int>(result == VK_SUCCESS, "failed to create the graphics command pool for frame {}", i);

        //: allocate the main command buffer used for rendering
        auto alloc = allocate_info;
        alloc.commandPool = frame.at(i).command_pool;
        result = vkAllocateCommandBuffers(gpu.device, &alloc, &frame.at(i).main_command_buffer);
        graphics_assert<int>(result == VK_SUCCESS, "failed to allocate the main graphics command buffer for frame {}", i);

        //: deletion queue
        dq.push([command_pool = frame.at(i).command_pool, device = gpu.device]{
            vkDestroyCommandPool(device, command_pool, nullptr);
        });
    });

    log::graphics("initialized {} graphics command pools and buffers", engine_config.vk_frames_in_flight());
}

// ··········
// · UPDATE ·
// ··········

void GraphicsSystem::update() {
    glfwPollEvents();
}

// ············
// · STOPPING ·
// ············

//* stop the vulkan api
void GraphicsSystem::stop() {
    //: clear the deletion queues in reverse order
    auto vk = const_cast<VulkanAPI*>(api.get());
    vk->deletion_queue_swapchain.clear();
    vk->deletion_queue_global.clear();

    //: stop glfw
    if (win) glfwDestroyWindow(win->window);
    glfwTerminate();
}

// ·········
// · DEBUG ·
// ·········

//* glfw error callback
void vk::glfwErrorCallback(int error, const char *description) {
    log::error("glfw error {}, {}", error, description);
}

#ifdef FRESA_DEBUG
//* vulkan debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL vk::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, ui64 obj, std::size_t location, int code, const char* layer_prefix, const char* msg, void* user_data) {
    str message = msg;

    //: pretty formatting
    std::size_t i_object, i_error, i_spec, i_web;
    i_object = message.find("Object 0", 0);
    if (i_object != std::string::npos)
        i_error = message.find("MessageID", i_object) + 25;
    if (i_error != std::string::npos)
        i_spec = message.find("The Vulkan spec states", i_error);
    if (i_spec != std::string::npos)
        i_web = message.find("(https://vulkan", i_spec);
    if (i_object != std::string::npos and i_error != std::string::npos and i_spec != std::string::npos and i_web != std::string::npos)
        message = fmt::format("{}\n  {}\n  {}\n  {}",
                              fmt::styled(message.substr(0, i_object), fmt::fg(fmt::color::pink)),
                              message.substr(i_error, i_spec - i_error),
                              fmt::styled(message.substr(i_spec, i_web - i_spec), fmt::emphasis::bold | fmt::fg(fmt::color::slate_gray)),
                              fmt::styled(message.substr(i_web), fmt::fg(fmt::color::slate_gray)));

    detail::log<"VK VALIDATION", LOG_GRAPHICS, fmt::color::hot_pink>("{}", message);
    return VK_FALSE;
}

//* create debug callback
VkDebugReportCallbackEXT vk::createDebugCallback(VkInstance instance, DeletionQueue& dq) {
    graphics_assert(vkCreateDebugReportCallbackEXT != nullptr, "vulkan debug callback function was not initialized");

    //: create info
    VkDebugReportCallbackCreateInfoEXT create_info{
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
        .pfnCallback = vk::debugCallback
    };

    //: create callback
    VkDebugReportCallbackEXT callback;
    VkResult result = vkCreateDebugReportCallbackEXT(instance, &create_info, nullptr, &callback);
    graphics_assert<std::size_t>(result == VK_SUCCESS, "fatal error creating a vulkan debug callback: {}", result);

    dq.push([instance, callback]{ vkDestroyDebugReportCallbackEXT(instance, callback, nullptr); });
    return callback;
}
#endif