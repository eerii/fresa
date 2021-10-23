//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include "r_vulkan.h"
#include "config.h"
#include "r_vertex.h"

#include <optional>

#define MAX_FRAMES_IN_FLIGHT 2

namespace Verse::Graphics
{
    struct Vulkan;

    namespace VK {
        void initVulkan(Vulkan *vulkan, Config &c);

        struct QueueFamilyIndices {
            std::optional<ui32> graphics_queue_family_index;
            std::optional<ui32> present_queue_family_index;
            std::optional<ui32> compute_queue_family_index;
            
            bool all() { return (graphics_queue_family_index.has_value() and present_queue_family_index.has_value() and
                                 compute_queue_family_index.has_value()); };
        };
    
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };
    
        struct RenderingCreateInfo {
            VkPipelineVertexInputStateCreateInfo vertex_input;
            VkPipelineInputAssemblyStateCreateInfo input_assembly;
            VkPipelineRasterizationStateCreateInfo rasterizer;
            VkPipelineMultisampleStateCreateInfo multisampling;
            VkPipelineDepthStencilStateCreateInfo depth_stencil;
            VkPipelineColorBlendAttachmentState color_blend_attachment;
            VkPipelineColorBlendStateCreateInfo color_blend_state;
            
            VkPipelineViewportStateCreateInfo viewport_state;
            VkViewport viewport;
            VkRect2D scissor;
            
            VkVertexInputBindingDescription vertex_input_binding_description;
            std::array<VkVertexInputAttributeDescription, 2> vertex_input_attribute_descriptions;
        };
    
        //TODO: CHANGE THIS
        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };
    }

    struct Vulkan {
        //DEVICE
        //----------------------------------------
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        void createInstance(Config &c);
        
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        ui16 ratePhysicalDevice(VkPhysicalDevice physical_device);
        void selectPhysicalDevice();
        
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkQueue compute_queue;
        
        VK::QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physical_device);
        VK::QueueFamilyIndices queue_families;
        
        void selectQueueFamily();
        
        VkDevice device;
        void createDevice();
        
        VkSurfaceKHR surface;
        void createSurface(Config &c);
        
        VkDebugReportCallbackEXT debug_callback;
        void createDebug();
        //----------------------------------------
        
        //SWAPCHAIN
        //----------------------------------------
        VK::SwapchainSupportDetails getSwapchainSupport(VkPhysicalDevice physical_device);
        VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D selectSwapExtent(Config &c, const VkSurfaceCapabilitiesKHR& capabilities);
        
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
        
        VkSwapchainKHR swapchain;
        void createSwapchain(Config &c);
        
        std::vector<VkImageView> swapchain_image_views;
        void createImageViews();
        VkImageView createImageView(VkImage image, VkImageAspectFlags aspect_flags);
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
        void createVertexBuffer(const std::vector<Graphics::Vertex> &vertices);
        void createIndexBuffer(const std::vector<ui16> &indices);
        
        void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
        
        ui32 getMemoryType(ui32 filter, VkMemoryPropertyFlags properties);
        
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_memory;
        ui32 index_buffer_size;
        
        
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        
        std::vector<VkBuffer> uniform_buffers;
        std::vector<VkDeviceMemory> uniform_buffers_memory;
        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;
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
        void renderFrame(Config &c);
        
        void updateUniformBuffer(ui32 current_image);
        //----------------------------------------
        
        //RECREATE SWAPCHAIN
        //----------------------------------------
        void recreateSwapchain(Config &c);
        //----------------------------------------
        
        //CLEANUP
        //----------------------------------------
        void destroySwapchain();
        void destroy();
        //----------------------------------------
    };
}

#endif
