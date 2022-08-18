//* vulkan_api
//      .

#include "r_api.h"
#include "config.h"
#include "strings.h"

using namespace fresa;
using namespace graphics;

namespace fresa::graphics::vk
{
    //: instance
    VkInstance createInstance();
    vk::GPU selectGPU(VkInstance instance);
    ui16 rateGPU(vk::GPU &gpu);

    //: debug
    void glfwErrorCallback(int error, const char* description);
    #ifdef FRESA_DEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type, ui64 obj, std::size_t location, int code, const char* layer_prefix, const char* msg, void* user_data);
    VkDebugReportCallbackEXT createDebugCallback(VkInstance instance);
    #endif
}

// ---
// INITIALIZATION
// ---

//* create vulkan graphics api
void GraphicsSystem::init() noexcept {
    //: initalize glfw
    glfwSetErrorCallback(vk::glfwErrorCallback);
    graphics_assert(glfwInit(), "failed to initalize glfw");

    //: check for vulkan support
	graphics_assert(glfwVulkanSupported(), "a vulkan loader has not been found");
    int version = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
    graphics_assert(version, "glad failed to load vulkan");
    log::graphics("glad vulkan loader ({}.{})", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    //: create vulkan api
    api = std::make_unique<VulkanAPI>();

    //: instance
    api->instance = vk::createInstance();
    version = gladLoaderLoadVulkan(api->instance, nullptr, nullptr);
    graphics_assert(version, "glad failed to load the vulkan functions that require an instance");
    api->debug_callback = vk::createDebugCallback(api->instance);

    //: gpu
    api->gpu = vk::selectGPU(api->instance);
    version = gladLoaderLoadVulkan(api->instance, api->gpu.gpu, nullptr);
    graphics_assert(version, "glad failed to load the extra vulkan extensions required by the gpu");
}

//* create vulkan instance
VkInstance vk::createInstance() {
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
    constexpr std::array<const char*, 1> validation_layers = { "VK_LAYER_KHRONOS_validation" };
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
    graphics_assert<int>(result == VK_SUCCESS, "fatal error creating a vulkan instance: {}", result);

    deletion_queues.global.push([instance]{ vkDestroyInstance(instance, nullptr); });
    log::graphics("created a vulkan instance");
    return instance;
}

//* rate physical device
//      give a score to the gpu based on its capabilities
//      if the score returned is 0, the gpu is invalid, otherwise selectGPU will choose the one with the highest score
ui16 vk::rateGPU(vk::GPU &gpu) {
    ui16 score = 16;

    //: prefer discrete gpus
    if (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 256;
    else if (gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 16;

    //: prefer higher memory (only for discrete gpus)
    auto heaps = std::vector<VkMemoryHeap>(gpu.memory.memoryHeaps, gpu.memory.memoryHeaps + gpu.memory.memoryHeapCount);
    for (const auto& heap : heaps)
        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT and gpu.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += heap.size / 1024 / 1024 / 128;

    //: required device extensions
    constexpr std::array<str_view, 2> required_extensions{
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    ui32 extension_count;
    vkEnumerateDeviceExtensionProperties(gpu.gpu, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(gpu.gpu, nullptr, &extension_count, available_extensions.data());
    for (const auto& ext : required_extensions) {
        auto it = std::find_if(available_extensions.begin(), available_extensions.end(), [&](const auto& e) { return e.extensionName == ext; });
        if (it == available_extensions.end()) { return 0; }
    }

    //- check queue families
    //- check swapchain support

    return score;
}

//* select physical device
vk::GPU vk::selectGPU(VkInstance instance) {
    //: count devices
    ui32 gpu_count;
    VkResult result = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    graphics_assert<int>(result == VK_SUCCESS, "fatal error enumerating physical devices: {}", result);
    graphics_assert(gpu_count > 0, "no physical devices found");
    
    //: get the available gpus
    std::vector<VkPhysicalDevice> physical_devices(gpu_count);
    vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data());
    log::graphics("gpu{}:", gpu_count > 1 ? "s" : "");

    //: fill the gpu list with all the relevant information
    std::vector<vk::GPU> gpus;
    for (auto d : physical_devices) {
        gpus.push_back(vk::GPU{ .gpu = d });
        vk::GPU &gpu = gpus.back();

        //: get properties
        vkGetPhysicalDeviceProperties(gpu.gpu, &gpu.properties);
        vkGetPhysicalDeviceFeatures(gpu.gpu, &gpu.features);
        vkGetPhysicalDeviceMemoryProperties(gpu.gpu, &gpu.memory);

        //: rate the gpu
        gpu.score = rateGPU(gpu);
    }

    //: get the gpu with the highest score
    auto it = std::max_element(gpus.begin(), gpus.end(), [](const auto& a, const auto& b) { return a.score < b.score; });
    graphics_assert(it != gpus.end() and it->score > 0, "no suitable gpu found");

    //: log the chosen gpu
    for (auto gpu : gpus)
        log::graphics(" {} {}", gpu.properties.deviceID == it->properties.deviceID ? "-" : "·", lower(gpu.properties.deviceName));

    return *it;
}

// ---
// STOPPING
// ---

//* stop the vulkan api
void GraphicsSystem::stop() {
    //: clear the deletion queues in reverse order
    deletion_queues.clear();

    //: stop glfw
    glfwTerminate();
}

// ---
// DEBUG
// ---

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
VkDebugReportCallbackEXT vk::createDebugCallback(VkInstance instance) {
    graphics_assert(vkCreateDebugReportCallbackEXT != nullptr, "vulkan debug callback function was not initialized");

    //: create info
    VkDebugReportCallbackCreateInfoEXT create_info{
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
        .pfnCallback = vk::debugCallback,
        .pUserData = api.get()
    };

    //: create callback
    VkDebugReportCallbackEXT callback;
    VkResult result = vkCreateDebugReportCallbackEXT(instance, &create_info, nullptr, &callback);
    graphics_assert<std::size_t>(result == VK_SUCCESS, "fatal error creating a vulkan debug callback: {}", result);

    deletion_queues.global.push([instance, callback]{ vkDestroyDebugReportCallbackEXT(instance, callback, nullptr); });
    return callback;
}
#endif