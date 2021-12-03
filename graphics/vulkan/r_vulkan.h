//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include "vk_mem_alloc.h"

#include "r_dtypes.h"

namespace Fresa::Graphics
{
    struct AttachmentData {
        AttachmentType type;
        TextureData texture;
        
        VkImageUsageFlagBits usage;
        VkImageAspectFlagBits aspect;
    };

    struct VkSwapchainData {
        VkFormat format;
        VkExtent2D extent;
        
        VkSwapchainKHR swapchain;
        
        ui32 size;
        
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        
        std::vector<VkFramebuffer> framebuffers;
        std::map<ui32, AttachmentData> attachments;
        
        VkRenderPass main_render_pass;
    };

    struct VkPipelineData {
        ShaderData shader;
        
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
        VkDescriptorSetLayout descriptor_layout;
        
        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
        std::vector<VkDescriptorPool> descriptor_pools;
        
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        
        ui32 subpass;
    };

    struct VkQueueIndices {
        std::optional<ui32> graphics;
        std::optional<ui32> present;
        std::optional<ui32> transfer;
        std::optional<ui32> compute;
    };

    struct VkQueueData {
        VkQueue graphics;
        VkQueue present;
        VkQueue transfer;
        VkQueue compute;
    };

    struct VkCommandData {
        std::map<str, VkCommandPool> command_pools;
        std::map<str, std::vector<VkCommandBuffer>> command_buffers;
        
        VkQueueIndices queue_indices;
        VkQueueData queues;
    };
    
    struct VkCommandPoolHelperData {
        std::optional<ui32> queue;
        std::optional<VkCommandPoolCreateFlagBits> flags;
    };

    struct VkSyncData {
        std::vector<VkSemaphore> semaphores_image_available;
        std::vector<VkSemaphore> semaphores_render_finished;
        
        std::vector<VkFence> fences_in_flight;
        std::vector<VkFence> fences_images_in_flight;
        
        ui8 current_frame;
    };

    struct VkSwapchainSupportData {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct VkRenderPassCreateData {
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        std::vector<VkAttachmentDescription> attachments;
    };

    struct VkPipelineCreateInfo {
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
        
        std::vector<VkVertexInputBindingDescription> vertex_input_binding_description;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
    };

    struct WriteDescriptorBuffer {
        VkDescriptorBufferInfo info;
        VkWriteDescriptorSet write;
    };

    struct WriteDescriptorImage {
        VkDescriptorImageInfo info;
        VkWriteDescriptorSet write;
    };

    struct Vulkan {
        //Device
        //----------------------------------------
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        
        VkSurfaceKHR surface;
        
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceFeatures physical_device_features;
        
        VkDevice device;
        //----------------------------------------
        
        
        //---Memory---
        VmaAllocator allocator;
        
        //---Swapchain---
        VkSwapchainData swapchain;
        
        //---Commands---
        VkCommandData cmd;
        
        //---Sync---
        VkSyncData sync;
        
        //---Pipelines---
        std::map<DrawShaders, VkPipelineData> draw_pipelines;
        
        //---Images---
        VkSampler sampler;
        
        //---Debug---
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


