//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include "vk_mem_alloc.h"

#include <vector>
#include <map>
#include <optional>
#include <array>
#include <glm/glm.hpp>

#include "r_bufferdata.h"
#include "r_shaderdata.h"

namespace Verse::Graphics
{
    struct VkSwapchainData {
        VkFormat format;
        VkExtent2D extent;
        
        VkSwapchainKHR swapchain;
        
        ui32 size;
        
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        
        VkImage depth_image;
        VkDeviceMemory depth_image_memory;
        VkImageView depth_image_view;
        
        VkRenderPass main_render_pass;
        
        std::vector<VkFramebuffer> framebuffers;
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

    struct VkSyncData {
        std::vector<VkSemaphore> semaphores_image_available;
        std::vector<VkSemaphore> semaphores_render_finished;
        
        std::vector<VkFence> fences_in_flight;
        std::vector<VkFence> fences_images_in_flight;
        
        ui8 current_frame;
    };

    struct VkDescriptorSetData {
        VkDescriptorSetLayout layout;
        std::vector<VkDescriptorPool> pools;
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
        
        
        //Pipeline
        //----------------------------------------
        ShaderData shader;
        
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        //----------------------------------------
        
        
        //---Descriptors---
        VkDescriptorSetData descriptors;
        
        
        //---Images---
        VkSampler sampler;
        
        
        //---Debug---
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


