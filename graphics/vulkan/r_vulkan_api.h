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
#include "r_drawdata.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Verse::Graphics::VK
{
    //Device
    //----------------------------------------
    VkInstance createInstance(const WindowData &win);

    VkSurfaceKHR createSurface(VkInstance instance, const WindowData &win);

    ui16 ratePhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
    VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice physical_device);
    VkFormat chooseSupportedFormat(VkPhysicalDevice physical_device, const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling, VkFormatFeatureFlags features);

    VkDevice createDevice(VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures physical_device_features,
                          const QueueIndices &queue_indices);
    QueueIndices getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    QueueData getQueues(VkDevice device, const QueueIndices &queue_indices);
    //----------------------------------------


    //Memory
    //----------------------------------------
    VmaAllocator createMemoryAllocator(VkDevice device, VkPhysicalDevice physical_device, VkInstance instance);
    //----------------------------------------


    //Swapchain
    //----------------------------------------
    SwapchainSupportData getSwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR> &modes);
    VkExtent2D selectSwapExtent(VkSurfaceCapabilitiesKHR capabilities, const WindowData &win);
    
    VkSwapchainData createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                    const QueueIndices &queue_indices, const WindowData &win);
    void recreateSwapchain(Vulkan &vk, const WindowData &win);

    VkFormat getDepthFormat(Vulkan &vk);
    void createDepthResources(Vulkan &vk);
    //----------------------------------------


    //Commands
    //----------------------------------------
    std::map<str, VkCommandPool> createCommandPools(VkDevice device, const QueueIndices &queue_indices, std::vector<str> keys,
                                                    std::map<str, ui32> queues, std::map<str, VkCommandPoolCreateFlagBits> flags);

    std::vector<VkCommandBuffer> allocateDrawCommandBuffers(VkDevice device, ui32 swapchain_size, const VkCommandData &cmd);
    void beginDrawCommandBuffer(VkCommandBuffer cmd, VkPipeline pipeline, VkFramebuffer framebuffer,
                                VkRenderPass render_pass, VkExtent2D extent);
    void recordDrawCommandBuffer(const Vulkan &vk, ui32 current);

    VkCommandBuffer beginSingleUseCommandBuffer(VkDevice device, VkCommandPool pool);
    void endSingleUseCommandBuffer(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool pool, VkQueue queue);
    //----------------------------------------


    //Render pass
    //----------------------------------------
    VkSubpassDescription createRenderSubpass();
    VkSubpassDependency createRenderSubpassDependency();
    VkAttachmentDescription createRenderPassAttachment(VkFormat format);
    RenderPassCreateData prepareRenderPass(VkFormat format);

    VkRenderPass createRenderPass(VkDevice device, VkFormat format);

    VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass render_pass, VkImageView image_view, VkExtent2D extent);
    std::vector<VkFramebuffer> createFramebuffers(VkDevice device, const VkSwapchainData &swapchain);
    //----------------------------------------


    //Sync objects
    //----------------------------------------
    VkSyncData createSyncObjects(VkDevice device, ui32 swapchain_size);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    ShaderData createShaderData(VkDevice device, str vert = "", str frag = "", str compute = "", str geometry = "");

    PipelineCreateInfo preparePipelineCreateInfo(VkExtent2D extent);
    VkPipelineVertexInputStateCreateInfo preparePipelineCreateInfoVertexInput(
        const std::vector<VkVertexInputBindingDescription> &binding, const std::vector<VkVertexInputAttributeDescription> &attributes);
    VkPipelineInputAssemblyStateCreateInfo preparePipelineCreateInfoInputAssembly();
    VkViewport preparePipelineCreateInfoViewport(VkExtent2D extent);
    VkRect2D preparePipelineCreateInfoScissor(VkExtent2D extent);
    VkPipelineViewportStateCreateInfo preparePipelineCreateInfoViewportState(const VkViewport &viewport, const VkRect2D &scissor);
    VkPipelineRasterizationStateCreateInfo preparePipelineCreateInfoRasterizer();
    VkPipelineMultisampleStateCreateInfo preparePipelineCreateInfoMultisampling();
    VkPipelineDepthStencilStateCreateInfo preparePipelineCreateInfoDepthStencil();
    VkPipelineColorBlendAttachmentState preparePipelineCreateInfoColorBlendAttachment();
    VkPipelineColorBlendStateCreateInfo preparePipelineCreateInfoColorBlendState(const VkPipelineColorBlendAttachmentState &attachment);
    
    VkPipelineLayout createPipelineLayout(VkDevice device, const VkDescriptorSetLayout &descriptor_set_layout);
    VkPipeline createGraphicsPipeline(VkDevice device, const VkPipelineLayout &layout,
                                      const VkSwapchainData &swapchain, const ShaderStages &stages);
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory);
    void copyBuffer(VkDevice device, const VkCommandData &cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    //----------------------------------------


    //Descriptors
    //----------------------------------------
    VkDescriptorSetLayoutBinding prepareDescriptorSetLayoutBinding(VkShaderStageFlagBits stage, VkDescriptorType type, ui32 binding);
    VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, const ShaderCode &code, ui32 swapchain_size);

    VkDescriptorPool createDescriptorPool(VkDevice device);

    std::vector<VkDescriptorSet> allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout,
                                                        std::vector<VkDescriptorPool> &pools, ui32 swapchain_size);

    WriteDescriptorBuffer createWriteDescriptorUniformBuffer(VkDescriptorSet descriptor_set, ui32 binding, BufferData uniform_buffer);
    WriteDescriptorImage createWriteDescriptorCombinedImageSampler(VkDescriptorSet descriptor_set, ui32 binding,
                                                                   VkImageView image_view, VkSampler sampler);
    void updateDescriptorSets(VkDevice device, const std::vector<VkDescriptorSet> &descriptor_sets, ui32 swapchain_size,
                              const std::vector<BufferData> &uniform_buffers);
    //----------------------------------------


    //Uniforms
    //----------------------------------------
    std::vector<BufferData> createUniformBuffers(VmaAllocator allocator, ui32 swapchain_size);

    template<typename T>
    void updateUniformBuffer(VmaAllocator allocator, BufferData buffer, T ubo) {
        void* data;
        vmaMapMemory(allocator, buffer.allocation, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vmaUnmapMemory(allocator, buffer.allocation);
    }
    //----------------------------------------


    //Images
    //----------------------------------------
    void createImage(Vulkan &vk, TextureData &tex, ui8 *pixels);
    void transitionImageLayout(Vulkan &vk, TextureData &tex, VkImageLayout new_layout);
    void copyBufferToImage(Vulkan &vk, BufferData &buffer, TextureData &tex);

    VkImageView createImageView(VkDevice device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    void createSampler(Vulkan &vk);
    //----------------------------------------


    //Render
    //----------------------------------------
    ui32 startRender(VkDevice device, const VkSwapchainData &swapchain, VkSyncData &sync, std::function<void()> recreate_swapchain);
    void renderFrame(Vulkan &vk, WindowData &win, ui32 index);
    //----------------------------------------


    //Debug
    //----------------------------------------
    VkDebugReportCallbackEXT createDebug(VkInstance &instance);
    //----------------------------------------
}

#endif
