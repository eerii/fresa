//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan.h"

#include "r_windowdata.h"
#include "r_vertexdata.h"
#include "r_bufferdata.h"
#include "r_texturedata.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Verse::Graphics::VK
{
    //Device
    //----------------------------------------
    void createInstance(Vulkan &vk, WindowData &win);

    void createSurface(Vulkan &vk, WindowData &win);

    ui16 ratePhysicalDevice(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    void selectPhysicalDevice(Vulkan &vk);

    QueueData getQueueFamilies(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    void selectQueueFamily(Vulkan &vk);

    void createDevice(Vulkan &vk);
    //----------------------------------------


    //Swapchain
    //----------------------------------------
    SwapchainSupportData getSwapchainSupport(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D selectSwapExtent(WindowData &win, const VkSurfaceCapabilitiesKHR& capabilities);
    
    void createSwapchain(Vulkan &vk, WindowData &win);
    void recreateSwapchain(Vulkan &vk, WindowData &win);

    void createImageViews(Vulkan &vk);

    VkFormat chooseSupportedFormat(Vulkan &vk, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat getDepthFormat(Vulkan &vk);

    void createDepthResources(Vulkan &vk);
    //----------------------------------------


    //Render Pass
    //----------------------------------------
    VkSubpassDescription createRenderSubpass();
    void createRenderPass(Vulkan &vk);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    void prepareRenderInfoVertexInput(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoInputAssembly(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoViewportState(RenderingCreateInfo &rendering_create_info, VkExtent2D extent);
    void prepareRenderInfoRasterizer(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoMultisampling(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoDepthStencil(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoColorBlendAttachment(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoColorBlendState(RenderingCreateInfo &rendering_create_info);
    void prepareRenderInfoPipelineLayout(RenderingCreateInfo &rendering_create_info);
    RenderingCreateInfo prepareRenderInfo(Vulkan &vk);
    
    void createDescriptorSetLayout(Vulkan &vk);
    void createPipelineLayout(Vulkan &vk);

    void createGraphicsPipeline(Vulkan &vk);
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(Vulkan &vk, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ui32 getMemoryType(Vulkan &vk, ui32 filter, VkMemoryPropertyFlags properties);

    void createVertexBuffer(Vulkan &vk, const std::vector<Graphics::VertexData> &vertices);
    void createIndexBuffer(Vulkan &vk, const std::vector<ui16> &indices);

    void createCommandBuffers(Vulkan &vk);
    void recordCommandBuffer(Vulkan &vk, VkCommandBuffer &buffer, VkFramebuffer &framebuffer, VkDescriptorSet &descriptor_set);

    VkCommandBuffer beginSingleUseCommandBuffer(Vulkan &vk);
    void endSingleUseCommandBuffer(Vulkan &vk, VkCommandBuffer command_buffer);

    void copyBuffer(Vulkan &vk, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    //----------------------------------------


    //Uniforms
    //----------------------------------------
    void createDescriptorPool(Vulkan &vk);
    void createDescriptorSets(Vulkan &vk);

    void createUniformBuffers(Vulkan &vk);
    void updateUniformBuffer(Vulkan &vk, ui32 current_image);
    //----------------------------------------


    //Images
    //----------------------------------------
    void createImage(Vulkan &vk, TextureData &tex, ui8 *pixels);
    void transitionImageLayout(Vulkan &vk, TextureData &tex, VkImageLayout new_layout);
    void copyBufferToImage(Vulkan &vk, BufferData &buffer, TextureData &tex);

    VkImageView createImageView(Vulkan &vk, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    VkSampler createSampler(Vulkan &vk);
    //----------------------------------------


    //Framebuffers
    //----------------------------------------
    void createFramebuffers(Vulkan &vk);
    //----------------------------------------


    //Command Pools
    //----------------------------------------
    void createCommandPools(Vulkan &vk);
    //----------------------------------------


    //Sync objects
    //----------------------------------------
    void createSyncObjects(Vulkan &vk);
    //----------------------------------------


    //Render
    //----------------------------------------
    void renderFrame(Vulkan &vk, WindowData &win);
    //----------------------------------------


    //Debug
    //----------------------------------------
    void createDebug(Vulkan &vk);
    //----------------------------------------

    //Cleanup
    //----------------------------------------
    void cleanSwapchain(Vulkan &vk);
    //----------------------------------------
}

#endif
