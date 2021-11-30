//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_api.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Fresa::Graphics::VK
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
                          const VkQueueIndices &queue_indices);
    VkQueueIndices getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    VkQueueData getQueues(VkDevice device, const VkQueueIndices &queue_indices);
    //----------------------------------------


    //Memory
    //----------------------------------------
    VmaAllocator createMemoryAllocator(VkDevice device, VkPhysicalDevice physical_device, VkInstance instance);
    //----------------------------------------


    //Swapchain
    //----------------------------------------
    VkSwapchainSupportData getSwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR> &modes);
    VkExtent2D selectSwapExtent(VkSurfaceCapabilitiesKHR capabilities, const WindowData &win);
    
    VkSwapchainData createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                    const VkQueueIndices &queue_indices, const WindowData &win);
    void recreateSwapchain(Vulkan &vk, const WindowData &win);
    //----------------------------------------


    //Commands
    //----------------------------------------
    std::map<str, VkCommandPool> createCommandPools(VkDevice device, const VkQueueIndices &queue_indices, std::vector<str> keys,
                                                    std::map<str, ui32> queues, std::map<str, VkCommandPoolCreateFlagBits> flags);

    std::vector<VkCommandBuffer> allocateDrawCommandBuffers(VkDevice device, ui32 swapchain_size, const VkCommandData &cmd);
    void beginDrawCommandBuffer(VkCommandBuffer cmd, VkPipeline pipeline, VkFramebuffer framebuffer,
                                VkRenderPass render_pass, VkExtent2D extent);
    void recordDrawCommandBuffer(const Vulkan &vk, ui32 current);

    VkCommandBuffer beginSingleUseCommandBuffer(VkDevice device, VkCommandPool pool);
    void endSingleUseCommandBuffer(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool pool, VkQueue queue);
    //----------------------------------------


    //Shaders
    //----------------------------------------
    VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
    ShaderStages createShaderStages(VkDevice device, const ShaderCode &code);

    std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfo(const ShaderStages &stages);
    void destroyShaderStages(VkDevice device, const ShaderStages &stages);

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        
        VkVertexInputBindingDescription v;
        v.binding = 0;
        v.stride = sizeof(V);
        v.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        binding_descriptions.push_back(v);
        
        return binding_descriptions;
    }

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attr_v = API::getAttributeDescriptions<V>();
        std::vector<VkVertexInputAttributeDescription> attr;
        attr.resize(attr_v.size());
        std::transform(attr_v.begin(), attr_v.end(), attr.begin(), [](const auto &a){
            VkVertexInputAttributeDescription v;
            v.binding = a.binding;
            v.location = a.location;
            v.offset = a.offset;
            switch (a.format) {
                case VERTEX_FORMAT_R_F:
                    v.format = VK_FORMAT_R32_SFLOAT; break;
                case VERTEX_FORMAT_RG_F:
                    v.format = VK_FORMAT_R32G32_SFLOAT; break;
                case VERTEX_FORMAT_RGB_F:
                    v.format = VK_FORMAT_R32G32B32_SFLOAT; break;
                case VERTEX_FORMAT_RGBA_F:
                    v.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
                default:
                    v.format = VK_FORMAT_R32_SFLOAT; break;
            }
            return v;
        });
        return attr;
        //Expand to allow multiple vertex descriptions, use it so the user can pass multiple
        //template arguments and they will be created for each of them
    }
    //----------------------------------------


    //Render pass
    //----------------------------------------
    VkSubpassDescription createRenderSubpass();
    VkSubpassDependency createRenderSubpassDependency();
    VkAttachmentDescription createRenderPassAttachment(VkFormat format);
    VkAttachmentDescription createRenderPassDepthAttachment(VkFormat format);
    VkRenderPassCreateData prepareRenderPass(VkFormat format, VkFormat depth_format);

    VkRenderPass createRenderPass(VkDevice device, VkFormat format, VkFormat depth_format);

    VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass render_pass, std::vector<VkImageView> attachments, VkExtent2D extent);
    std::vector<VkFramebuffer> createFramebuffers(VkDevice device, const VkSwapchainData &swapchain);
    //----------------------------------------


    //Sync objects
    //----------------------------------------
    VkSyncData createSyncObjects(VkDevice device, ui32 swapchain_size);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    VkPipelineCreateInfo preparePipelineCreateInfo(VkExtent2D extent);
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
    std::pair<VkImage, VmaAllocation> createImage(VkDevice device, VmaAllocator allocator, VmaMemoryUsage memory,
                                                  Vec2<> size, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage);
    void transitionImageLayout(VkDevice device, const VkCommandData &cmd, TextureData &tex, VkImageLayout new_layout);
    void copyBufferToImage(VkDevice device, const VkCommandData &cmd, BufferData &buffer, TextureData &tex);

    VkImageView createImageView(VkDevice device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    VkSampler createSampler(VkDevice device);
    //----------------------------------------


    //Depth
    //----------------------------------------
    VkFormat getDepthFormat(VkPhysicalDevice physical_device);
    bool hasDepthStencilComponent(VkFormat format);
    TextureData createDepthTexture(VkDevice device, VmaAllocator allocator, VkPhysicalDevice physical_device,
                                   const VkCommandData &cmd, Vec2<> size);
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
