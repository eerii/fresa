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
    VkInstance createInstance(WindowData &win);

    VkSurfaceKHR createSurface(VkInstance &instance, WindowData &win);

    ui16 ratePhysicalDevice(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    VkPhysicalDevice selectPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface);
    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice &physical_device);
    VkFormat chooseSupportedFormat(VkPhysicalDevice &physical_device, const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling, VkFormatFeatureFlags features);

    VkDevice createDevice(VkPhysicalDevice &physical_device, VkPhysicalDeviceFeatures &physical_device_features, QueueIndices &queue_indices);
    QueueIndices getQueueFamilies(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    QueueData getQueues(VkDevice &device, QueueIndices &queue_indices);
    //----------------------------------------


    //Swapchain
    //----------------------------------------
    SwapchainSupportData getSwapchainSupport(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(SwapchainSupportData &support);
    VkPresentModeKHR selectSwapPresentMode(SwapchainSupportData &support);
    VkExtent2D selectSwapExtent(SwapchainSupportData &support, WindowData &win);
    
    VkSwapchainData createSwapchain(VkDevice &device, VkPhysicalDevice &physical_device, VkSurfaceKHR &surface,
                                    QueueIndices &queue_indices, WindowData &win);
    void recreateSwapchain(Vulkan &vk, WindowData &win);

    VkFormat getDepthFormat(Vulkan &vk);
    void createDepthResources(Vulkan &vk);
    //----------------------------------------


    //Commands
    //----------------------------------------
    std::map<str, VkCommandPool> createCommandPools(VkDevice &device, QueueIndices &queue_indices, std::vector<str> keys,
                                                    std::map<str, ui32> queues, std::map<str, VkCommandPoolCreateFlagBits> flags);

    std::vector<VkCommandBuffer> createDrawCommandBuffers(VkDevice &device, VkSwapchainData &swapchain, VkCommandData &cmd);
    void recordDrawCommandBuffers(Vulkan &vk);
    //----------------------------------------


    //Render pass
    //----------------------------------------
    VkSubpassDescription createRenderSubpass();
    VkSubpassDependency createRenderSubpassDependency();
    VkAttachmentDescription createRenderPassAttachment(VkFormat format);
    RenderPassCreateData prepareRenderPass(VkSwapchainData &swapchain);

    VkRenderPass createRenderPass(VkDevice &device, VkSwapchainData &swapchain);

    VkFramebuffer createFramebuffer(VkDevice &device, VkRenderPass &render_pass, VkImageView &image_view, VkExtent2D &extent);
    std::vector<VkFramebuffer> createFramebuffers(VkDevice &device, VkSwapchainData &swapchain);
    //----------------------------------------


    //Sync objects
    //----------------------------------------
    VkSyncData createSyncObjects(VkDevice &device, VkSwapchainData &swapchain);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    ShaderData createShaderData(VkDevice &device, str vert = "", str frag = "", str compute = "", str geometry = "");

    VkDescriptorSetLayoutBinding prepareDescriptorSetLayoutBinding(VkShaderStageFlagBits stage, VkDescriptorType type, ui32 binding);
    VkDescriptorSetLayout createDescriptorSetLayout(VkDevice &device, ShaderData &shader);

    PipelineCreateInfo preparePipelineCreateInfo(VkExtent2D extent);
    VkPipelineVertexInputStateCreateInfo preparePipelineCreateInfoVertexInput(
        std::vector<VkVertexInputBindingDescription> &binding, std::vector<VkVertexInputAttributeDescription> &attributes);
    VkPipelineInputAssemblyStateCreateInfo preparePipelineCreateInfoInputAssembly();
    VkViewport preparePipelineCreateInfoViewport(VkExtent2D extent);
    VkRect2D preparePipelineCreateInfoScissor(VkExtent2D extent);
    VkPipelineViewportStateCreateInfo preparePipelineCreateInfoViewportState(VkViewport &viewport, VkRect2D &scissor);
    VkPipelineRasterizationStateCreateInfo preparePipelineCreateInfoRasterizer();
    VkPipelineMultisampleStateCreateInfo preparePipelineCreateInfoMultisampling();
    VkPipelineDepthStencilStateCreateInfo preparePipelineCreateInfoDepthStencil();
    VkPipelineColorBlendAttachmentState preparePipelineCreateInfoColorBlendAttachment();
    VkPipelineColorBlendStateCreateInfo preparePipelineCreateInfoColorBlendState(VkPipelineColorBlendAttachmentState &attachment);
    
    VkPipelineLayout createPipelineLayout(VkDevice &device, VkDescriptorSetLayout &descriptor_set_layout);
    VkPipeline createGraphicsPipeline(VkDevice &device, VkPipelineLayout &layout, VkSwapchainData &swapchain, ShaderStages &stages);
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(Vulkan &vk, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ui32 getMemoryType(Vulkan &vk, ui32 filter, VkMemoryPropertyFlags properties);

    void createVertexBuffer(Vulkan &vk, const std::vector<Graphics::VertexData> &vertices);
    void createIndexBuffer(Vulkan &vk, const std::vector<ui16> &indices);

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

    VkImageView createImageView(VkDevice &device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    void createSampler(Vulkan &vk);
    //----------------------------------------


    //Render
    //----------------------------------------
    void renderFrame(Vulkan &vk, WindowData &win);
    //----------------------------------------


    //Debug
    //----------------------------------------
    VkDebugReportCallbackEXT createDebug(VkInstance &instance);
    //----------------------------------------

    //Cleanup
    //----------------------------------------
    void cleanSwapchain(Vulkan &vk);
    //----------------------------------------
}

#endif
