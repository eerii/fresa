//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef USE_VULKAN

#include "r_common_api.h"

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
    
    struct PipelineCreateData {
        std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
        VkPipelineInputAssemblyStateCreateInfo input_assembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineDepthStencilStateCreateInfo depth_stencil;
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineLayout layout;
        ShaderID shader;
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
        std::map<str, std::array<VkCommandBuffer, Config::frames_in_flight>> command_buffers;
        
        VkQueueIndices queue_indices;
        VkQueueData queues;
        
        ui8 current_image;
    };

    struct SyncData {
        std::array<VkSemaphore, Config::frames_in_flight> semaphores_image_available;
        std::array<VkSemaphore, Config::frames_in_flight> semaphores_render_finished;
        
        std::array<VkFence, Config::frames_in_flight> fences_in_flight;
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
        
        //: Image sampler
        VkSampler sampler;
        
        //: Window vertex buffer
        BufferData window_vertex_buffer;
        
        //: GUI
        IF_GUI(RenderPassID gui_render_pass;)
        
        //: Debug
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


