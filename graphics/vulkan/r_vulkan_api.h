//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef USE_VULKAN

#include "r_api.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Fresa::Graphics::VK
{
    //Deletion queues
    //----------------------------------------
    inline std::vector<std::function<void()>> deletion_queue_program;
    inline std::vector<std::function<void()>> deletion_queue_size_change;
    inline std::vector<std::function<void()>> deletion_queue_swapchain;
    inline std::vector<std::function<void()>> deletion_queue_frame;
    //----------------------------------------

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
    
    SwapchainData createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                  const VkQueueIndices &queue_indices, const WindowData &win);
    void recreateSwapchain(Vulkan &vk, const WindowData &win);
    //----------------------------------------


    //Commands
    //----------------------------------------
    std::map<str, VkCommandPool> createCommandPools(VkDevice device, const VkQueueIndices &queue_indices,
                                                    std::map<str, VkCommandPoolHelperData> data);

    std::vector<VkCommandBuffer> allocateDrawCommandBuffers(VkDevice device, ui32 swapchain_size, const CommandData &cmd);
    void beginDrawCommandBuffer(VkCommandBuffer cmd, const RenderData &render, ui32 index);
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


    //Attachments
    //----------------------------------------
    VkAttachmentDescription createAttachmentDescription(const AttachmentData &attachment);
    AttachmentID registerAttachment(const Vulkan &vk, std::map<AttachmentID, AttachmentData> &attachments, AttachmentType type);
    void recreateAttachments(VkDevice device, VmaAllocator allocator, VkPhysicalDevice physical_device,
                             const CommandData &cmd, Vec2<> size, std::map<AttachmentID, AttachmentData> &attachments);

    VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass render_pass, std::vector<VkImageView> attachments, VkExtent2D extent);
    std::vector<VkFramebuffer> createFramebuffers(VkDevice device, const RenderData &render);
    //----------------------------------------


    //Render pass
    //----------------------------------------
    SubpassID registerSubpass(RenderData &render, std::vector<AttachmentID> ids);
    VkRenderPass createRenderPass(VkDevice device, const RenderData &render);
    //----------------------------------------


    //Sync objects
    //----------------------------------------
    SyncData createSyncObjects(VkDevice device, ui32 swapchain_size);
    //----------------------------------------


    //Descriptors
    //----------------------------------------
    VkDescriptorSetLayoutBinding prepareDescriptorSetLayoutBinding(VkShaderStageFlagBits stage, VkDescriptorType type, ui32 binding);
    std::vector<VkDescriptorSetLayoutBinding> createDescriptorSetLayoutBindings(VkDevice device, const ShaderCode &code, ui32 swapchain_size);
    VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &bindings);

    std::vector<VkDescriptorPoolSize> createDescriptorPoolSizes(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    VkDescriptorPool createDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize> &sizes);

    std::vector<VkDescriptorSet> allocateDescriptorSets(VkDevice device, VkDescriptorSetLayout layout,
                                                        const std::vector<VkDescriptorPoolSize> &sizes,
                                                        std::vector<VkDescriptorPool> &pools, ui32 swapchain_size);

    WriteDescriptorBuffer createWriteDescriptorUniformBuffer(VkDescriptorSet descriptor_set, ui32 binding, BufferData uniform_buffer);
    WriteDescriptorImage createWriteDescriptorCombinedImageSampler(VkDescriptorSet descriptor_set, ui32 binding,
                                                                   VkImageView image_view, VkSampler sampler);
    WriteDescriptorImage createWriteDescriptorInputAttachment(VkDescriptorSet descriptor_set, ui32 binding, VkImageView image_view);

    void updateDescriptorSets(const Vulkan &vk, const std::vector<VkDescriptorSet> &descriptor_sets,
                              const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                              std::map<ui32, const std::vector<BufferData>*> uniform_buffers = {},
                              std::map<ui32, const VkImageView*> image_views = {},
                              std::map<ui32, const VkImageView*> input_attachments = {});
    void updatePostDescriptorSets(const Vulkan &vk, const PipelineData &pipeline);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    VkPipelineHelperData preparePipelineCreateInfo(VkExtent2D extent, const std::vector<VkVertexInputBindingDescription> binding_description,
                                                   const std::vector<VkVertexInputAttributeDescription> attribute_description);
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
    VkPipeline createGraphicsPipelineObject(VkDevice device, const PipelineData &data, const RenderData &render);
    void recreatePipeline(const Vulkan &vk, PipelineData &data);

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    PipelineData createPipeline(const Vulkan &vk, Shaders shader, SubpassID subpass) {
        PipelineData data;
        
        //---Subpass---
        data.subpass = subpass;
        log::graphics("Pipeline %s, subpass %d", shader_names.at(shader).c_str(), data.subpass);
        log::graphics("---");
        
        //---Shader data---
        data.shader = API::createShaderData(shader_names.at(shader));
        data.shader.stages = VK::createShaderStages(vk.device, data.shader.code);
        
        //---Descriptor pool---
        data.descriptor_layout_bindings = VK::createDescriptorSetLayoutBindings(vk.device, data.shader.code, vk.render.swapchain.size);
        data.descriptor_layout = VK::createDescriptorSetLayout(vk.device, data.descriptor_layout_bindings);
        data.descriptor_pool_sizes = VK::createDescriptorPoolSizes(data.descriptor_layout_bindings);
        data.descriptor_pools.push_back(VK::createDescriptorPool(vk.device, data.descriptor_pool_sizes));
        
        //---Descriptor sets---
        if (data.subpass > 0) {
            data.descriptor_sets = VK::allocateDescriptorSets(vk.device, data.descriptor_layout, data.descriptor_pool_sizes,
                                                              data.descriptor_pools, vk.render.swapchain.size);
            VK::updatePostDescriptorSets(vk, data);
        }
        
        //---Binding and attribute descriptors---
        data.binding_descriptions = VK::getBindingDescriptions<V>();
        data.attribute_descriptions = VK::getAttributeDescriptions<V>();
        
        //---Pipeline---
        data.pipeline_layout = VK::createPipelineLayout(vk.device, data.descriptor_layout);
        data.pipeline = VK::createGraphicsPipelineObject(vk.device, data, vk.render);
        
        return data;
    }
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory);
    void copyBuffer(VkDevice device, const CommandData &cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<ui16> &indices);

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    BufferData createVertexBuffer(const GraphicsAPI &api, const std::vector<V> &vertices) {
        //---Vertex buffer---
        //      Buffer that holds the vertex information for the shaders to use.
        //      It has a struct per vertex of the mesh, which can contain properties like position, color, uv, normals...
        //      The properties are described automatically using reflection in an attribute description in the pipeline
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
        
        //: Staging buffer
        //      We want to make the vertex buffer accessible to the GPU in the most efficient way, so we use a staging buffer
        //      This is created in CPU only memory, in which we can easily map the vertex data
        BufferData staging_buffer = VK::createBuffer(api.allocator, buffer_size,
                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VMA_MEMORY_USAGE_CPU_ONLY);
        
        //: Map vertices to staging buffer
        void* data;
        vmaMapMemory(api.allocator, staging_buffer.allocation, &data);
        memcpy(data, vertices.data(), (size_t) buffer_size);
        vmaUnmapMemory(api.allocator, staging_buffer.allocation);
        
        //: Vertex buffer
        //      The most efficient memory for GPU access is VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, or its rough equivalent, VMA_MEMORY_USAGE_GPU_ONLY
        //      This memory type can't be access from the CPU
        BufferData vertex_buffer = VK::createBuffer(api.allocator, buffer_size,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VMA_MEMORY_USAGE_GPU_ONLY);
        
        //: Copy from staging to vertex
        //      Since we can't access the vertex buffer memory from the CPU, we will use vkCmdCopyBuffer(), which will execute on a queue
        //      and move data between the staging and vertex buffer
        VK::copyBuffer(api.device, api.cmd, staging_buffer.buffer, vertex_buffer.buffer, buffer_size);
        
        //: Delete helpers (staging now, vertex when the program finishes)
        vmaDestroyBuffer(api.allocator, staging_buffer.buffer, staging_buffer.allocation);
        deletion_queue_program.push_back([api, vertex_buffer](){
            vmaDestroyBuffer(api.allocator, vertex_buffer.buffer, vertex_buffer.allocation); 
        });
        
        return vertex_buffer;
    }
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
    TextureData createTexture(VkDevice device, VmaAllocator allocator, VkPhysicalDevice physical_device,
                              VkImageUsageFlagBits usage, VkImageAspectFlagBits aspect, Vec2<> size, VkFormat format, Channels ch);
    std::pair<VkImage, VmaAllocation> createImage(VkDevice device, VmaAllocator allocator, VmaMemoryUsage memory,
                                                  Vec2<> size, VkFormat format, VkImageLayout layout, VkImageUsageFlags usage);
    void transitionImageLayout(VkDevice device, const CommandData &cmd, TextureData &tex, VkImageLayout new_layout);
    void copyBufferToImage(VkDevice device, const CommandData &cmd, BufferData &buffer, TextureData &tex);

    VkImageView createImageView(VkDevice device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    VkSampler createSampler(VkDevice device);
    //----------------------------------------


    //Depth
    //----------------------------------------
    VkFormat getDepthFormat(VkPhysicalDevice physical_device);
    bool hasDepthStencilComponent(VkFormat format);
    //----------------------------------------


    //Render
    //----------------------------------------
    ui32 startRender(VkDevice device, const SwapchainData &swapchain, SyncData &sync, std::function<void()> recreate_swapchain);
    void renderFrame(Vulkan &vk, WindowData &win, ui32 index);
    //----------------------------------------


    //Debug
    //----------------------------------------
    VkDebugReportCallbackEXT createDebug(VkInstance &instance);
    inline Vec2<> to_vec(VkExtent2D extent) {
        return Vec2<>(extent.width, extent.height);
    }
    //----------------------------------------
}

namespace Fresa::Graphics::API {
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    DrawBufferID registerDrawBuffer(const GraphicsAPI &api, const std::vector<V> &vertices, const std::vector<ui16> &indices) {
        static DrawBufferID id = 0;
        do id++;
        while (draw_buffer_data.find(id) != draw_buffer_data.end());
        
        draw_buffer_data[id] = DrawBufferData{};
        
        draw_buffer_data[id].vertex_buffer = VK::createVertexBuffer(api, vertices);
        draw_buffer_data[id].index_buffer = VK::createIndexBuffer(api, indices);
        draw_buffer_data[id].index_size = (ui32)indices.size();
        
        return id;
    }
}

#endif
