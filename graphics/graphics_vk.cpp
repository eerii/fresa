//* graphics_api
//      .

#include "graphics_api.h"
#include "config.h"
#include "fresa_assert.h"
#include "strings.h"

using namespace fresa;

namespace fresa::vk
{
    VkInstance createInstance();
}

//* create vulkan graphics api
void GraphicsSystem::init() noexcept {
    //: initalize glfw
    fresa_assert(glfwInit(), "failed to initalize glfw");

    //: check for vulkan support
	fresa_assert(glfwVulkanSupported(), "a vulkan loader has not been found");
    int version = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
    fresa_assert(version, "glad failed to load vulkan");
    log::graphics("glad vulkan loader ({}.{})", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    //: instance
    VkInstance instance = vk::createInstance();
}

//* create vulkan instance
VkInstance vk::createInstance() {
    //: instance extensions
    //      platform specific extensions needed to create the window surface
    ui32 instance_extension_count = 0;
    const char** instance_extension_buffer = glfwGetRequiredInstanceExtensions(&instance_extension_count);
    std::vector<const char*> instance_extensions(instance_extension_count);
    log::graphics("instance extensions:");
    for (ui32 i = 0; i < instance_extension_count; i++) {
        instance_extensions.at(i) = instance_extension_buffer[i];
        log::graphics(" - {}", lower(instance_extensions.at(i)));
    }

    //: validation layers
    //      middleware for existing vulkan functionality
    //      primarily used for getting detailed error descriptions, in this case with VK_LAYER_KHRONOS_validation
    //      enabled using the engine config option enable_validation_layers()
    constexpr std::array<const char*, 1> validation_layers = { "VK_LAYER_KHRONOS_validation" };
    if constexpr (engine_config.enable_validation_layers()) {
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
    }

    //: application info
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = str(engine_config.name()).c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(engine_config.version().at(0), engine_config.version().at(1), engine_config.version().at(2));
    app_info.pEngineName = "fresa";
    app_info.engineVersion = VK_MAKE_VERSION(EngineConfig{}.version().at(0), EngineConfig{}.version().at(1), EngineConfig{}.version().at(2));
    app_info.apiVersion = VK_API_VERSION_1_1;

    //: instance info
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = (int)instance_extensions.size();
    create_info.ppEnabledExtensionNames = instance_extensions.data();
    
    if constexpr (engine_config.enable_validation_layers()) {
        create_info.enabledLayerCount = (int)validation_layers.size();
        create_info.ppEnabledLayerNames = validation_layers.data();
    }

    //: create instance
    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    fresa_assert<std::size_t>(result != VK_SUCCESS, "fatal error creating a vulkan instance: {}", result);

    //- deletion_queue
    return instance;
}