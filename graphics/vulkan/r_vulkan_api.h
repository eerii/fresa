//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef USE_VULKAN

#include "r_api.h"

namespace Fresa::Graphics::VK
{
    //Queues
    //----------------------------------------
    inline std::vector<std::function<void()>> deletion_queue_program;
    inline std::vector<std::function<void()>> deletion_queue_swapchain;
    inline std::vector<std::function<void()>> deletion_queue_frame;
    //----------------------------------------

    //Device
    //----------------------------------------
    VkInstance createInstance();

    VkSurfaceKHR createSurface(VkInstance instance);

    ui16 ratePhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
                                          VkPhysicalDeviceFeatures &features, VkPhysicalDeviceProperties &properties);
    VkFormat chooseSupportedFormat(VkPhysicalDevice physical_device, const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling, VkFormatFeatureFlags features);
    VkSampleCountFlagBits getMaxMSAASamples(VkPhysicalDeviceProperties &properties);

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
    VkExtent2D selectSwapExtent(VkSurfaceCapabilitiesKHR capabilities);
    
    SwapchainData createSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkQueueIndices &queue_indices);
    void recreateSwapchain();
    //----------------------------------------


    //Commands
    //----------------------------------------
    std::map<str, VkCommandPool> createCommandPools(VkDevice device, const VkQueueIndices &queue_indices,
                                                    std::map<str, VkCommandPoolHelperData> data);

    std::array<VkCommandBuffer, Config::frames_in_flight> allocateDrawCommandBuffers();
    void recordRenderCommandBuffer();
    void setViewport(const VkCommandBuffer &cmd, VkExtent2D extent);

    VkCommandBuffer beginSingleUseCommandBuffer(VkDevice device, VkCommandPool pool);
    void endSingleUseCommandBuffer(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool pool, VkQueue queue);
    
    #ifdef DEBUG
    VkQueryPool createQueryPool(VkQueryType type);
    std::vector<ui64> getQueryResults(VkQueryPool query, ui32 offset, ui32 count);
    #endif
    //----------------------------------------


    //Shaders
    //----------------------------------------
    std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfo(const ShaderPass &pass);
    //----------------------------------------


    //Attachments
    //----------------------------------------
    VkSampleCountFlagBits getAttachmentSamples(const Vulkan &vk, const AttachmentData &attachment);
    VkAttachmentDescription createAttachmentDescription(const AttachmentData &attachment, VkSampleCountFlagBits samples);
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
    SyncData createSyncObjects();
    //----------------------------------------


    //Descriptors
    //----------------------------------------
    VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    
    void updateDescriptorSets(ShaderID shader, std::vector<VkBuffer> uniform_buffers = {}, std::vector<VkBuffer> storage_buffers = {},
                              std::vector<VkImageView> image_views = {}, std::vector<VkImageView> input_attachments = {});
    void linkPipelineDescriptors(ShaderID shader);
    
    template <VkDescriptorType type, typename T>
    void prepareWriteDescriptor(WriteDescriptors &descriptors, ShaderID shader, ui32 &count, std::vector<T> data, std::vector<ui32> bindings = {}) {
        if (data.size() == 0) return;
        
        constexpr bool is_buffer = type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        constexpr bool is_image = type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER or type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        
        for (const auto &d : Shader::getShader(shader).descriptors) {
            int index = -1;
            for (const auto &binding : d.bindings) {
                if ((ShaderDescriptorT)binding.descriptor_type != type)
                    continue;
                
                if (bindings.size() == 0) {
                    index++;
                } else {
                    auto it = std::find(bindings.begin(), bindings.end(), binding.binding);
                    if (it == bindings.end())
                        continue;
                    index = (int)std::distance(bindings.begin(), it);
                }
                
                for (int i = 0; i < Config::frames_in_flight; i++) {
                    if constexpr (is_buffer) {
                        descriptors.buffer[count].buffer = data.at(index);
                        descriptors.buffer[count].offset = 0;
                        descriptors.buffer[count].range = VK_WHOLE_SIZE;
                    }
                    
                    if constexpr (is_image) {
                        descriptors.image[count].imageView = data.at(index);
                        descriptors.image[count].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        descriptors.image[count].sampler = type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? api.sampler : VK_NULL_HANDLE;
                    }
                    
                    descriptors.write[count].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptors.write[count].dstSet = d.descriptors.at(i);
                    descriptors.write[count].dstBinding = binding.binding;
                    descriptors.write[count].dstArrayElement = 0;
                    descriptors.write[count].descriptorType = type;
                    descriptors.write[count].descriptorCount = 1;
                    descriptors.write[count].pBufferInfo = is_buffer ? &descriptors.buffer[count] : nullptr;
                    descriptors.write[count].pImageInfo = is_image ? &descriptors.image[count] : nullptr;
                    descriptors.write[count].pTexelBufferView = nullptr;
                    
                    count++;
                }
            }
        }
    }
    
    //----------------------------------------
    
    
    //Buffers
    //----------------------------------------
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    BufferData createVertexBuffer(const std::vector<V> &vertices) {
        //---Vertex buffer---
        //      Buffer that holds the vertex information for the shaders to use.
        //      It has a struct per vertex of the mesh, which can contain properties like position, color, uv, normals...
        //      The properties are described automatically using reflection in an attribute description in the pipeline
        return Common::allocateBuffer(ui32(sizeof(V) * vertices.size()), BUFFER_USAGE_VERTEX, BUFFER_MEMORY_GPU_ONLY, (void*)vertices.data());
    }
    
    template <typename I, std::enable_if_t<std::is_integral_v<I>, bool> = true>
    BufferData createIndexBuffer(const std::vector<I> &indices) {
        //---Index buffer---
        //      This buffer contains a list of indices, which allows to draw complex meshes without repeating vertices
        //      A simple example, while a square only has 4 vertices, 6 vertices are needed for the 2 triangles, and it only gets worse from there
        //      An index buffer solves this by having a list of which vertices to use, avoiding vertex repetition
        //      The creating process is very similar to the above vertex buffer, using a staging buffer
        return Common::allocateBuffer(ui32(sizeof(I) * indices.size()), BUFFER_USAGE_INDEX, BUFFER_MEMORY_GPU_ONLY, (void*)indices.data());
    }
    
    template <typename T>
    void updateBuffer(BufferData &buffer, const std::vector<T> &v, size_t offset = 0) {
        
    }
    
    template <typename T>
    void updateGPUBuffer(BufferData &buffer, const std::vector<T> &v, size_t offset = 0) {
        //: Create a staging buffer
        ui32 buffer_size = ui32(sizeof(T) * v.size());
        BufferData staging_buffer = Common::allocateBuffer(buffer_size, BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_CPU_ONLY, nullptr);
        
        //: Copy data to the staging buffer
        void* data;
        vmaMapMemory(api.allocator, staging_buffer.allocation, &data);
        memcpy(data, v.data(), (size_t)buffer_size);
        vmaUnmapMemory(api.allocator, staging_buffer.allocation);
        
        //: Make the GPU copy from one buffer to another
        Common::copyBuffer(staging_buffer, buffer, buffer_size, ui32(sizeof(T) * offset));
        
        //: Destroy the staging buffer
        Common::destroyBuffer(staging_buffer);
        buffer_list.erase(staging_buffer);
    }
    
    void updateBufferFromCompute(const BufferData &buffer, ui32 buffer_size, ShaderID shader);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout> &set_layouts, const std::vector<VkPushConstantRange> &push_constants);
    VkPipeline buildGraphicsPipeline(const PipelineCreateData &data, ShaderID shader);
    
    namespace Pipeline {
        //: Helper functions
        VkPipelineInputAssemblyStateCreateInfo getInputAssembly(VkPrimitiveTopology topology);
        VkPipelineRasterizationStateCreateInfo getRasterizer(VkPolygonMode polygon_mode, VkCullModeFlagBits cull, float line_width = 1.0f);
        VkPipelineMultisampleStateCreateInfo getMultisampling();
        VkPipelineColorBlendAttachmentState getColorBlend();
        VkPipelineDepthStencilStateCreateInfo getDepthStencil(bool depth_test, bool depth_write, VkCompareOp compare_op);
        VkVertexInputBindingDescription getVertexBindingDescription(ui32 size, ui32 binding, VkVertexInputRate rate);
        
        //: Configuration data
        struct ConfigData {
            std::vector<std::pair<str, VertexInputRate>> vertex_descriptions;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
            VkCullModeFlagBits cull = VK_CULL_MODE_NONE;
            float line_width = 1.0f;
            bool depth_test = true;
            bool depth_write = true;
            VkCompareOp compare_op = VK_COMPARE_OP_LESS;
        };
        
        //: Get pipeline data
        PipelineCreateData getCreateData(ConfigData config, ShaderID shader);
    };
    
    //----------------------------------------


    //Images
    //----------------------------------------
    TextureData createTexture(VkDevice device, VmaAllocator allocator, VkPhysicalDevice physical_device,
                              VkImageUsageFlagBits usage, VkImageAspectFlagBits aspect, Vec2<ui16> size, VkFormat format, Channels ch);
    std::pair<VkImage, VmaAllocation> createImage(VkDevice device, VmaAllocator allocator, VmaMemoryUsage memory, Vec2<ui16> size,
                                                  VkSampleCountFlagBits samples, ui32 mip_levels, VkFormat format,
                                                  VkImageLayout layout, VkImageUsageFlags usage);
    void transitionImageLayout(VkDevice device, VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
    void transitionImageLayoutCmd(VkDevice device, const CommandData &cmd, TextureData &tex, VkImageLayout new_layout);
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
    ui8 startRender();
    void renderFrame();
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
        void init(Vulkan &vk);
        void transferFonts(const Vulkan &vk);
        void recordGuiCommandBuffer();
    }
    #endif
    //----------------------------------------
}

namespace Fresa::Graphics {
    inline Vec2<> to_vec(VkExtent2D extent) {
        return Vec2<>(extent.width, extent.height);
    }
    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    inline VkExtent2D to_extent(Vec2<T> vec) {
        return VkExtent2D{(ui32)vec.x, (ui32)vec.y};
    }
    inline bool operator ==(const VkExtent2D &a, const VkExtent2D &b) {
        return a.width == b.width and a.height == b.height;
    }
    inline bool operator ==(const VkExtent2D &a, const Vec2<> &b) {
        return a.width == b.x and a.height == b.y;
    }
    
    template <typename... UBO>
    void updateComputeUniformBuffers(ShaderID shader, const UBO& ...ubo) {
        //TODO: ENABLE COMPUTE SHADERS
        log::warn("Enable compute shaders");
        /*int i = 0;
        ([&](){
            updateUniformBuffer(api, api.compute_pipelines.at(shader).uniform_buffers.at(i++).at(0), ubo);
        }(), ...);*/
    }
    
    template <typename V>
    void updateBufferFromCompute(const BufferData &buffer, ui32 buffer_size,
                                 ShaderID shader, std::function<std::vector<V>()> fallback) {
        static_assert(sizeof(V) % sizeof(glm::vec4) == 0, "The buffer should be aligned to a vec4 (4 floats) for the compute shader padding to match");
        #ifdef HAS_COMPUTE
            VK::updateBufferFromCompute(buffer, buffer_size, shader);
        #else
            static_assert(false, "The Vulkan renderer should have compute capabilities, check if you are setting the correct macro");
        #endif
    }
}

#endif
