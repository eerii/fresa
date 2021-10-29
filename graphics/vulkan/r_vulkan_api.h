//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan.h"

#include "r_windowdata.h"
#include "r_vertexdata.h"

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

    namespace Swapchain
    {
    //Swapchain
    //----------------------------------------
    SwapchainSupportData getSwapchainSupport(VkSurfaceKHR &surface, VkPhysicalDevice &physical_device);
    
    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D selectSwapExtent(WindowData &win, const VkSurfaceCapabilitiesKHR& capabilities);
    
    void createSwapchain(Vulkan &vk, WindowData &win);
    
    VkImageView createImageView(VkDevice &device, VkImage image, VkImageAspectFlags aspect_flags, VkFormat format);
    void createImageViews(Vulkan &vk);
    
    void recreateSwapchain(Vulkan &vk, WindowData &win);
    //----------------------------------------
    }

    namespace Debug
    {
    void createDebug(Vulkan &vk);
    }
}

namespace Verse::Graphics
{
    struct VulkanOld {
        
        
        
        //----------------------------------------
        
        //PIPELINE
        //----------------------------------------
        VkRenderPass render_pass;
        void createRenderPass();
        VkSubpassDescription createRenderSubpass();
        
        VK::RenderingCreateInfo rendering_create_info;
        void prepareRenderInfoVertexInput();
        void prepareRenderInfoInputAssembly();
        void prepareRenderInfoViewportState();
        void prepareRenderInfoRasterizer();
        void prepareRenderInfoMultisampling();
        void prepareRenderInfoDepthStencil();
        void prepareRenderInfoColorBlendAttachment();
        void prepareRenderInfoColorBlendState();
        void prepareRenderInfoPipelineLayout();
        void prepareRenderInfo();
        
        VkDescriptorSetLayout descriptor_set_layout;
        void createDescriptorSetLayout();
        
        VkPipelineLayout pipeline_layout;
        void createPipelineLayout();
        
        VkPipeline pipeline;
        void createGraphicsPipeline();
        //----------------------------------------
        
        //BUFFERS
        //----------------------------------------
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
        void createVertexBuffer(const std::vector<Graphics::VertexData> &vertices);
        void createIndexBuffer(const std::vector<ui16> &indices);
        
        void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
        
        ui32 getMemoryType(ui32 filter, VkMemoryPropertyFlags properties);
        
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;
        ui32 index_buffer_size;
        //----------------------------------------
        
        //RENDERING
        //----------------------------------------
        std::vector<VkFramebuffer> swapchain_framebuffers;
        void createFramebuffers();
        
        VkCommandPool command_pool;
        VkCommandPool temp_command_pool;
        void createCommandPools();
        
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
