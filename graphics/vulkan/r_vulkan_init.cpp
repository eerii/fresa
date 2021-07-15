//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

void Verse::Graphics::VK::initVulkan(Vulkan *vulkan, Config &c) {
    vulkan->createInstance(c);
    vulkan->createDebug();
    
    vulkan->createSurface(c);
    
    vulkan->selectPhysicalDevice();
    vulkan->selectQueueFamily();
    vulkan->createDevice();
    
    vulkan->createSwapchain(c);
    vulkan->createImageViews();
    
    vulkan->createRenderPass();
    vulkan->createGraphicsPipeline();
    
    vulkan->createFramebuffers();
    vulkan->createCommandPools();
    vulkan->createCommandBuffers();
    
    vulkan->createSyncObjects();
}

#endif
