//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan.h"

#include "r_windowdata.h"
#include "r_vertexdata.h"
#include "r_bufferdata.h"

#define MAX_FRAMES_IN_FLIGHT 2

namespace Verse::Graphics::VK
{
    namespace Init
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
    }


    //Swapchain
    //----------------------------------------
    namespace Swapchain
    {
    SwapchainSupportData getSwapchainSupport(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D selectSwapExtent(WindowData &win, const VkSurfaceCapabilitiesKHR& capabilities);
    }

    void createSwapchain(Vulkan &vk, WindowData &win);
    void recreateSwapchain(Vulkan &vk, WindowData &win);

    VkImageView createImageView(VkDevice &device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    void createImageViews(Vulkan &vk);
    //----------------------------------------


    //Render Pass
    //----------------------------------------
    VkSubpassDescription createRenderSubpass();
    void createRenderPass(Vulkan &vk);
    //----------------------------------------


    //Pipeline
    //----------------------------------------
    namespace Pipeline
    {
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
    }

    void createGraphicsPipeline(Vulkan &vk);
    //----------------------------------------


    //Buffers
    //----------------------------------------
    BufferData createBuffer(Vulkan &vk, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void copyBuffer(Vulkan &vk, VkBuffer src, VkBuffer dst, VkDeviceSize size);
    ui32 getMemoryType(Vulkan &vk, ui32 filter, VkMemoryPropertyFlags properties);

    void createVertexBuffer(Vulkan &vk, const std::vector<Graphics::VertexData> &vertices);
    void createIndexBuffer(Vulkan &vk, const std::vector<ui16> &indices);
    //----------------------------------------


    //Framebuffers
    //----------------------------------------
    void createFramebuffers(Vulkan &vk);
    //----------------------------------------


    //Command Pools
    //----------------------------------------
    void createCommandPools(Vulkan &vk);
    //----------------------------------------


    //Debug
    //----------------------------------------
    void createDebug(Vulkan &vk);
    //----------------------------------------
}

namespace Verse::Graphics
{
    struct VulkanOld {
        
        //RENDERING
        //----------------------------------------
        
        
        
        
        
        std::vector<VkCommandBuffer> command_buffers;
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer &buffer, VkFramebuffer &framebuffer, VkDescriptorSet &descriptor_set);
        
        std::vector<VkSemaphore> semaphores_image_available;
        std::vector<VkSemaphore> semaphores_render_finished;
        std::vector<VkFence> fences_in_flight;
        std::vector<VkFence> fences_images_in_flight;
        void createSyncObjects();
        
        ui8 current_frame = 0;
        void renderFrame(WindowData &win);
        //----------------------------------------
        
        //UNIFORMS
        //----------------------------------------
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        
        std::vector<VkBuffer> uniform_buffers;
        std::vector<VkDeviceMemory> uniform_buffers_memory;
        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;
        
        void updateUniformBuffer(ui32 current_image);
        //----------------------------------------
        
        //CLEANUP
        //----------------------------------------
        void cleanSwapchain();
        void clean();
        //----------------------------------------
    };
}

#endif
