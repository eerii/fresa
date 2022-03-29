//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef USE_VULKAN

#include "r_common_api.h"
#include <SDL2/SDL_vulkan.h>

#define HAS_COMPUTE
#define MAX_WRITE_DESCRIPTORS 32

namespace Fresa::Graphics
{
    struct SwapchainData {
        //---Swapchain---
        //      The swapchain is responsible for presenting images to the screen. It holds several images that will swap at appropiate times
        VkFormat format;
        VkExtent2D extent;
        
        VkSwapchainKHR swapchain;
        
        ui32 size;
        ui32 min_image_count;
        
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
    };

    struct PipelineData {
        //---Pipeline---
        //      A pipeline is the device that has all the draw stages, such as vertex, fragment, geometry, rasterization...
        //      It can be configured through a series of descriptions
        //      We create one pipeline for each shader that we use
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
        VkDescriptorSetLayout descriptor_layout;
        
        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
        std::vector<VkDescriptorPool> descriptor_pools;
        
        std::vector<VkDescriptorSet> descriptor_sets;
        
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        
        std::vector<std::vector<BufferData>> uniform_buffers;
        
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        
        std::array<ui32, 3> group_size;
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

    struct CommandData {
        std::map<str, VkCommandPool> command_pools;
        std::map<str, std::vector<VkCommandBuffer>> command_buffers;
        
        VkQueueIndices queue_indices;
        VkQueueData queues;
        
        ui32 current_buffer;
    };

    struct SyncData {
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

    struct VkRenderPassHelperData {
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        std::vector<VkAttachmentDescription> attachments;
    };

    struct VkCommandPoolHelperData {
        std::optional<ui32> queue;
        std::optional<VkCommandPoolCreateFlagBits> flags;
    };

    struct VkPipelineHelperData {
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
    
    struct WriteDescriptors {
        std::array<VkDescriptorBufferInfo, MAX_WRITE_DESCRIPTORS> buffer;
        std::array<VkDescriptorImageInfo, MAX_WRITE_DESCRIPTORS> image;
        std::array<VkWriteDescriptorSet, MAX_WRITE_DESCRIPTORS> write;
    };
    
    struct Vulkan : CommonAPI {
        //: Instance
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        
        //: Surface
        VkSurfaceKHR surface;
        
        //: Physical device
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceFeatures physical_device_features;
        VkPhysicalDeviceProperties physical_device_properties;
        
        //: Logical device
        VkDevice device;
        
        //: Allocator
        VmaAllocator allocator;
        
        //: Render data
        SwapchainData swapchain;
        
        //: Commands
        CommandData cmd;
        SyncData sync;
        
        //: Pipelines
        std::map<ShaderID, PipelineData> pipelines;
        std::map<ShaderID, PipelineData> compute_pipelines;
        
        //: Image sampler
        VkSampler sampler;
        
        //: Window vertex buffer
        BufferData window_vertex_buffer;
        
        //: GUI
        IF_GUI(RenderPassID gui_render_pass;)
        
        //: Debug
        #ifdef DEBUG
        VkQueryPool query_timestamp;
        VkQueryPool query_statistics;
        #endif
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


