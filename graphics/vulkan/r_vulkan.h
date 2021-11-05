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

#include "r_vulkan_datatypes.h"
#include "r_bufferdata.h"
#include "r_texturedata.h"
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

    struct VkCommandData {
        std::map<str, VkCommandPool> command_pools;
        std::map<str, std::vector<VkCommandBuffer>> command_buffers;
        
        VK::QueueIndices queue_indices;
        VK::QueueData queues;
    };

    struct VkSyncData {
        std::vector<VkSemaphore> semaphores_image_available;
        std::vector<VkSemaphore> semaphores_render_finished;
        
        std::vector<VkFence> fences_in_flight;
        std::vector<VkFence> fences_images_in_flight;
        
        ui8 current_frame;
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
        
        //Buffers
        //----------------------------------------
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_buffer_size;
        //----------------------------------------
        
        
        //Uniforms
        //----------------------------------------
        std::vector<BufferData> uniform_buffers;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSetLayout descriptor_set_layout;
        std::vector<VkDescriptorSet> descriptor_sets;
        //----------------------------------------
        
        
        //Images
        //----------------------------------------
        TextureData test_image;
        VkImageView image_view;
        VkSampler sampler;
        //----------------------------------------
        
        
        //---Debug---
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


