//* vulkan_api
//      this defines the datatypes and functions used to interact with the vulkan api
//      included by r_api.h as a vulkan implementation of the graphics api
#pragma once

//: glad vulkan loader
#include <glad/vulkan.h>

//: vulkan memory allocator (load funcions dinamically)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#include "std_types.h"
#include "fresa_config.h"
#include <stack>

namespace fresa::graphics::vk
{
    // ····················
    // · VULKAN DATATYPES ·
    // ····················

    //: indices of the queue families
    enum struct QueueIndices {
        GRAPHICS,
        PRESENT,
        TRANSFER,
        COMPUTE,
        COUNT
    };

    //: physical device, representation of a gpu
    //      contains key vulkan structures, such as the gpu (physical device) and the logical device, which are used in almost every function
    //      there is also some important information about the gpu capabilities and extensions, as well as a list of available queues
    struct GPU {
        VkPhysicalDevice gpu = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;

        std::array<int, (ui32)QueueIndices::COUNT> queue_indices = {-1, -1, -1, -1};
        std::vector<VkQueue> queues;

        int score = -1;
    };

    //: swapchain
    //      it is what we use to present images to the screen
    //      must be recreated when the window changes
    struct Swapchain {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        VkFormat format = VK_FORMAT_UNDEFINED;
        VkExtent2D extent = {0, 0};
        
        ui32 size = 0;
        
        std::vector<VkImageView> image_views;
        std::vector<VkImage> images;
        std::vector<VkFence> fences_images_in_flight;
    };

    //: deletion queue
    //      called in reversed order of insertion to delete all resources without dependency issues
    struct DeletionQueue {
        std::stack<std::function<void()>> queue;
        void push(std::function<void()> &&f) { queue.push(f); }
        void clear() { while (!queue.empty()) { queue.top()(); queue.pop(); } }
    };

    //: per frame data
    //      since we can have multiple frames in flight (for ex. one being worked on by the cpu and one being presented by the gpu)
    //      we need to have some resources that are unique to each frame, such as command pools and syncronization objects
    struct FrameData {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        VkCommandBuffer main_command_buffer = VK_NULL_HANDLE;

        VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
        VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
        VkFence fence_in_flight = VK_NULL_HANDLE;

        DeletionQueue deletion_queue_frame;
    };

    //: pipeline configuration data
    //      includes a manageable subset of options for configuring a vulkan pipeline
    struct PipelineConfig {
        str_view name = "unnamed";
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
        VkCullModeFlagBits cull_mode = VK_CULL_MODE_NONE;
        VkCompareOp compare_op = VK_COMPARE_OP_LESS;
        float line_width = 1.0f;
        bool depth_test = true;
        bool depth_write = true;
    };

    // ·····················
    // · VULKAN API OBJECT ·
    // ·····················
    
    struct VulkanAPI {
        //: initialization structures
        VkInstance instance = VK_NULL_HANDLE;
        vk::GPU gpu;
        
        //: memory allocator (using vma)
        VmaAllocator allocator = VK_NULL_HANDLE;

        //: presentation
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        vk::Swapchain swapchain;

        //: per frame data
        std::array<vk::FrameData, engine_config.vk_frames_in_flight()> frame;

        //: descriptors
        std::vector<VkDescriptorPool> descriptor_pools;

        //: deletion queues
        vk::DeletionQueue deletion_queue_global;
        vk::DeletionQueue deletion_queue_swapchain;

        //: debug
        VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;
    };
}

namespace fresa::graphics
{
    // ·····················
    // · VULKAN INTERFACES ·
    // ·····················

    //: shader interfaces (used in r_shaders.h)
    using IShaderModule = VkShaderModule;
    using IDescriptorSetLayout = VkDescriptorSetLayout;
    using IDescriptorSet = VkDescriptorSet;
    using IDescriptorPool = VkDescriptorPool;
    using IPipelineLayout = VkPipelineLayout;
    using IPipeline = VkPipeline;

    constexpr auto shader_stage_values = std::to_array({
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_COMPUTE_BIT
    });

    constexpr auto shader_descriptor_values = std::to_array({
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    });

    //: render graph interfaces (used in r_render_graphs.h)
    using IImage = VkImage;
    using IImageView = VkImageView;
    using IImageUsageFlags = VkImageUsageFlags;
    using IImageAspectFlags = VkImageAspectFlags;
    using IImageLayout = VkImageLayout;
    using IFormat = VkFormat;
    using IAllocation = VmaAllocation;
    using IMemoryUsage = VmaMemoryUsage;
    using IAttachmentLoadOp = VkAttachmentLoadOp;
    using IAttachmentStoreOp = VkAttachmentStoreOp;
    using IAttachmentDescription = VkAttachmentDescription;

    constexpr auto no_image_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
    constexpr auto no_format = VK_FORMAT_MAX_ENUM;
    constexpr auto no_usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
    constexpr auto no_aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    constexpr auto no_memory_usage = VMA_MEMORY_USAGE_MAX_ENUM;
}