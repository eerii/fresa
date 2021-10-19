//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

namespace {
    //TODO: TEMPORAL, DELETE
    const std::vector<Graphics::Vertex> vertices = {
        {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        /*{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},*/
    };

    const std::vector<ui16> indices = {
        0, 1, 2, 2, 3, 0
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

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
    vulkan->createVertexBuffer(vertices);
    vulkan->createIndexBuffer(indices);
    vulkan->createCommandBuffers();
    
    vulkan->createSyncObjects();
}

#endif
