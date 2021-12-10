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
        //---Attachment---
        //      An attachment is an image the renderer writes to, it can either be the one in the swapchain that is presented
        //      or an intermediary that is used by a subpass.
        AttachmentType type;
        
        VkImage image;
        VmaAllocation allocation;
        
        VkImageView image_view;
        
        VkFormat format;
        
        VkImageUsageFlagBits usage;
        VkImageAspectFlagBits aspect;
        
        VkImageLayout initial_layout;
        VkImageLayout final_layout;
        
        VkAttachmentLoadOp load_op;
        VkAttachmentStoreOp store_op;
        
        VkAttachmentDescription description;
    };

    struct SwapchainData {
        //---Swapchain---
        //      The swapchain is responsible for presenting images to the screen. It holds several images that will swap at appropiate times
        VkFormat format;
        VkExtent2D extent;
        
        VkSwapchainKHR swapchain;
        
        ui32 size;
        
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        
        std::vector<VkFramebuffer> framebuffers;
    };

    struct SubpassData {
        //---Subpass---
        //      There can be multiple subpasses in a render pass, they perform commands (in our case shader and draw operations) on a range
        //      of associated attachments. They are used for post processing and other effects
        VkSubpassDescription description;
        
        std::vector<AttachmentID> attachment_bindings;
        std::vector<VkAttachmentReference> color_attachments;
        std::vector<VkAttachmentReference> depth_attachments;
        std::vector<VkAttachmentReference> input_attachments;
        
        std::map<AttachmentID, SubpassID> previous_subpass_dependencies;
    };

    struct RenderData {
        //---Render data---
        //      Struct that packs the important render data together for ease of use
        SwapchainData swapchain;
        std::map<AttachmentID, AttachmentData> attachments;
        VkRenderPass render_pass;
        std::vector<SubpassData> subpasses;
    };

    struct PipelineData {
        //---Pipeline---
        //      A pipeline is the device that has all the draw stages, such as vertex, fragment, geometry, rasterization...
        //      It can be configured through a series of descriptions
        //      We create one pipeline for each shader that we use
        ShaderData shader;
        
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
        VkDescriptorSetLayout descriptor_layout;
        
        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes;
        std::vector<VkDescriptorPool> descriptor_pools;
        
        std::vector<VkDescriptorSet> descriptor_sets;
        
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        
        SubpassID subpass;
    };

    //: Helper structs
    //----------------------------------------
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
    //----------------------------------------

    struct Vulkan {
        //: Instance
        VkInstance instance;
        std::vector<VkExtensionProperties> instance_extensions;
        
        //: Surface
        VkSurfaceKHR surface;
        
        //: Physical device
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceFeatures physical_device_features;
        
        //: Logical device
        VkDevice device;
        
        //: Allocator
        VmaAllocator allocator;
        
        //: Render data
        RenderData render;
        
        //: Commands
        CommandData cmd;
        SyncData sync;
        
        //: Pipelines
        std::map<Shaders, PipelineData> pipelines;
        
        //: Image sampler
        VkSampler sampler;
        
        //: Debug
        VkDebugReportCallbackEXT debug_callback;
    };
}


#endif


