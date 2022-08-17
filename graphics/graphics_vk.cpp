//* graphics_api
//      .

#include "graphics_api.h"
#include "fresa_assert.h"

using namespace fresa;

void GraphicsSystem::init() noexcept {
    //* initalize glfw
    fresa_assert(glfwInit(), "failed to initalize glfw");

    //* check for vulkan support
	fresa_assert(glfwVulkanSupported(), "a vulkan loader has not been found");
    int version = gladLoaderLoadVulkan(nullptr, nullptr, nullptr);
    fresa_assert(version, "glad failed to load vulkan");
    log::graphics("glad vulkan loader ({}.{})", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    
}