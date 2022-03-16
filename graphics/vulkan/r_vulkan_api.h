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
    VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDeviceFeatures &features);
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
    void recordRenderCommandBuffer(const Vulkan &vk, ui32 current);
    IndirectDrawID registerIndirectDrawCommandBuffer(Vulkan &vk, InstancedBufferID buffer);

    VkCommandBuffer beginSingleUseCommandBuffer(VkDevice device, VkCommandPool pool);
    void endSingleUseCommandBuffer(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool pool, VkQueue queue);
    
    #ifdef DEBUG
    VkQueryPool createQueryPool(VkDevice device, ui32 swapchain_size, VkQueryType type);
    std::vector<ui64> getQueryResults(VkDevice device, VkQueryPool query, ui32 offset, ui32 count);
    #endif
    //----------------------------------------


    //Shaders
    //----------------------------------------
    VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);
    ShaderStages createShaderStages(VkDevice device, const ShaderCode &code);

    std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfo(const ShaderStages &stages);
    void destroyShaderStages(VkDevice device, const ShaderStages &stages);

    template <typename... V>
    std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        ui32 binding = 0;
        
        ([&](){
            VkVertexInputBindingDescription v;
            
            if (binding == 0) v.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //: Regular vertex buffer
            else if (binding == 1) v.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE; //: Per instance vertex buffer
            else log::error("Currently only two vertex buffers are supported, per vertex and per instance");
            
            v.binding = binding++;
            v.stride = sizeof(V);
            
            binding_descriptions.push_back(v);
        }(), ...);
        
        return binding_descriptions;
    }

    template <typename... V>
    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VertexAttributeDescription> attr_v = API::getAttributeDescriptions<V...>();
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
    VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass render_pass, std::vector<VkImageView> attachments, VkExtent2D extent);
    std::vector<VkFramebuffer> createFramebuffers(VkDevice device, RenderPassID render_pass, VkExtent2D extent, const SwapchainData &swapchain);
    //----------------------------------------


    //Render pass
    //----------------------------------------
    RenderPassData createRenderPass(const Vulkan &vk, RenderPassID r_id);
    void recreateRenderPasses(Vulkan &vk);
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

    void updateDescriptorSets(const Vulkan &vk, const std::vector<VkDescriptorSet> &descriptor_sets,
                              const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                              std::map<ui32, const std::vector<BufferData>*> uniform_buffers = {},
                              std::map<ui32, VkImageView> image_views = {},
                              std::map<ui32, VkImageView> input_attachments = {});
    void updatePostDescriptorSets(const Vulkan &vk, const PipelineData &pipeline, ShaderID shader);
    //----------------------------------------
    
    
    //Uniforms
    //----------------------------------------
    std::vector<BufferData> createUniformBuffers(VmaAllocator allocator, ui32 swapchain_size, ui32 buffer_size);
    std::vector<std::vector<BufferData>> createPostUniformBuffers(const Vulkan &vk, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    VkPipelineHelperData preparePipelineCreateInfo(const std::vector<VkVertexInputBindingDescription> binding_description, const std::vector<VkVertexInputAttributeDescription> attribute_description, VkExtent2D extent);
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
    VkPipeline createGraphicsPipelineObject(const Vulkan &vk, const PipelineData &data, ShaderID shader);
    void recreatePipeline(const Vulkan &vk, PipelineData &data, ShaderID shader);

    template <typename... V>
    PipelineData createPipeline(const Vulkan &vk, ShaderID shader, SubpassID subpass) {
        PipelineData data;
        
        //---Subpass---
        API::Map::subpass_shader.add(subpass, shader);
        log::graphics("Pipeline %s, subpass %d", shader.c_str(), subpass);
        log::graphics("---");
        
        //---Shader data---
        API::shaders.at(shader).stages = VK::createShaderStages(vk.device, API::shaders.at(shader).code);
        
        //---Descriptor pool---
        data.descriptor_layout_bindings = VK::createDescriptorSetLayoutBindings(vk.device, API::shaders.at(shader).code, vk.swapchain.size);
        data.descriptor_layout = VK::createDescriptorSetLayout(vk.device, data.descriptor_layout_bindings);
        data.descriptor_pool_sizes = VK::createDescriptorPoolSizes(data.descriptor_layout_bindings);
        data.descriptor_pools.push_back(VK::createDescriptorPool(vk.device, data.descriptor_pool_sizes));
        
        //---Descriptor sets---
        if (not API::shaders.at(shader).is_draw) {
            data.descriptor_sets = VK::allocateDescriptorSets(vk.device, data.descriptor_layout, data.descriptor_pool_sizes,
                                                              data.descriptor_pools, vk.swapchain.size);
            data.uniform_buffers = VK::createPostUniformBuffers(vk, data.descriptor_layout_bindings);
            VK::updatePostDescriptorSets(vk, data, shader);
        }
        
        //---Binding and attribute descriptors---
        data.binding_descriptions = VK::getBindingDescriptions<V...>();
        data.attribute_descriptions = VK::getAttributeDescriptions<V...>();
        
        //---Pipeline---
        data.pipeline_layout = VK::createPipelineLayout(vk.device, data.descriptor_layout);
        data.pipeline = VK::createGraphicsPipelineObject(vk, data, shader);
        
        return data;
    }
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory);
    void copyBuffer(VkDevice device, const CommandData &cmd, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    
    template <typename V>
    BufferData createGPUBuffer(const GraphicsAPI &api, const std::vector<V> &v, VkBufferUsageFlags usage) {
        
        VkDeviceSize buffer_size = sizeof(V) * v.size();
        
        //: Staging buffer
        //      We want to make the vertex buffer accessible to the GPU in the most efficient way, so we use a staging buffer
        //      This is created in CPU only memory, in which we can easily map the vertex data
        BufferData staging_buffer = VK::createBuffer(api.allocator, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,  VMA_MEMORY_USAGE_CPU_ONLY);
        
        //: Map data to staging buffer
        void* data;
        vmaMapMemory(api.allocator, staging_buffer.allocation, &data);
        memcpy(data, v.data(), (size_t) buffer_size);
        vmaUnmapMemory(api.allocator, staging_buffer.allocation);
        
        //: Buffer
        //      The most efficient memory for GPU access is VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, or its rough equivalent, VMA_MEMORY_USAGE_GPU_ONLY
        //      This memory type can't be access from the CPU
        BufferData buffer = VK::createBuffer(api.allocator, buffer_size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
        
        //: Copy from staging to buffer
        //      Since we can't access the buffer memory from the CPU, we will use vkCmdCopyBuffer(), which will execute on a queue
        //      and move data between the staging and vertex buffer
        VK::copyBuffer(api.device, api.cmd, staging_buffer.buffer, buffer.buffer, buffer_size);
        
        //: Delete helpers (staging now, buffer when the program finishes)
        vmaDestroyBuffer(api.allocator, staging_buffer.buffer, staging_buffer.allocation);
        deletion_queue_program.push_back([api, buffer](){
            vmaDestroyBuffer(api.allocator, buffer.buffer, buffer.allocation);
        });
        
        return buffer;
    }

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    BufferData createVertexBuffer(const GraphicsAPI &api, const std::vector<V> &vertices) {
        //---Vertex buffer---
        //      Buffer that holds the vertex information for the shaders to use.
        //      It has a struct per vertex of the mesh, which can contain properties like position, color, uv, normals...
        //      The properties are described automatically using reflection in an attribute description in the pipeline
        return createGPUBuffer(api, vertices, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    
    template <typename I, std::enable_if_t<std::is_integral_v<I>, bool> = true>
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<I> &indices) {
        //---Index buffer---
        //      This buffer contains a list of indices, which allows to draw complex meshes without repeating vertices
        //      A simple example, while a square only has 4 vertices, 6 vertices are needed for the 2 triangles, and it only gets worse from there
        //      An index buffer solves this by having a list of which vertices to use, avoiding vertex repetition
        //      The creating process is very similar to the above vertex buffer, using a staging buffer
        return createGPUBuffer(api, indices, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
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
    void renderFrame(Vulkan &vk, WindowData &win);
    //----------------------------------------


    //Debug
    //----------------------------------------
    VkDebugReportCallbackEXT createDebug(VkInstance &instance);
    //----------------------------------------
    
    //Gui
    //----------------------------------------
    #ifndef DISABLE_GUI
    namespace Gui
    {
        inline VkDescriptorPool descriptor_pool;
        void init(Vulkan &vk, const WindowData &win);
        void transferFonts(const Vulkan &vk);
        void recordGuiCommandBuffer(const Vulkan &vk, ui32 current);
    }
    #endif
    //----------------------------------------
}

namespace Fresa::Graphics {
    inline Vec2<> to_vec(VkExtent2D extent) {
        return Vec2<>(extent.width, extent.height);
    }
    inline VkExtent2D to_extent(Vec2<> vec) {
        return VkExtent2D{(ui32)vec.x, (ui32)vec.y};
    }
    inline bool operator ==(const VkExtent2D &a, const VkExtent2D &b) {
        return a.width == b.width and a.height == b.height;
    }
    inline bool operator ==(const VkExtent2D &a, const Vec2<> &b) {
        return a.width == b.x and a.height == b.y;
    }
}

namespace Fresa::Graphics::API {
    template <typename UBO>
    DrawUniformID registerDrawUniforms(GraphicsAPI &api, ShaderID shader) {
        static DrawUniformID id = 0;
        do id++;
        while (draw_uniform_data.find(id) != draw_uniform_data.end());
        
        draw_uniform_data[id] = DrawUniformData{};
        DrawUniformData &data = draw_uniform_data.at(id);
        
        data.descriptor_sets = VK::allocateDescriptorSets(api.device, api.pipelines.at(shader).descriptor_layout,
                                                          api.pipelines.at(shader).descriptor_pool_sizes,
                                                          api.pipelines.at(shader).descriptor_pools, api.swapchain.size);
        data.uniform_buffers = VK::createUniformBuffers(api.allocator, api.swapchain.size, sizeof(UBO));
        data.size = (ui16)sizeof(UBO);
        
        return id;
    }
    
    template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    GeometryBufferID registerGeometryBuffer(const GraphicsAPI &api, const std::vector<V> &vertices, const std::vector<I> &indices) {
        static GeometryBufferID id = 0;
        do id++;
        while (geometry_buffer_data.find(id) != geometry_buffer_data.end());
        
        geometry_buffer_data[id] = GeometryBufferData{};
        GeometryBufferData &data = geometry_buffer_data.at(id);
        
        data.vertex_buffer = VK::createVertexBuffer(api, vertices);
        data.index_buffer = VK::createIndexBuffer(api, indices);
        data.index_size = (ui32)indices.size();
        data.index_bytes = (ui8)sizeof(I);
        
        return id;
    }
    
    template <typename V, typename U, typename I,
              std::enable_if_t<Reflection::is_reflectable<V> && Reflection::is_reflectable<U> && std::is_integral_v<I>, bool> = true>
    InstancedBufferID registerInstancedBuffer(const GraphicsAPI &api, const std::vector<V> &vertices,
                                              const std::vector<U> &instanced_data, const std::vector<I> &indices) {
        static InstancedBufferID id = 0;
        do id++;
        while (instanced_buffer_data.find(id) != instanced_buffer_data.end());
        
        instanced_buffer_data[id] = InstancedBufferData{};
        InstancedBufferData &data = instanced_buffer_data.at(id);
        
        data.vertex_buffer = VK::createVertexBuffer(api, vertices);
        
        data.index_buffer = VK::createIndexBuffer(api, indices);
        data.index_size = (ui32)indices.size();
        data.index_bytes = (ui8)sizeof(I);
        
        data.instance_buffer = VK::createVertexBuffer(api, instanced_data);
        data.instance_count = (ui32)instanced_data.size();
        
        return id;
    }
}

#endif
