//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <optional>
#include <array>
#include <glm/glm.hpp>

namespace Verse::Graphics::VK
{
    struct QueueData {
        std::optional<ui32> graphics_index;
        std::optional<ui32> present_index;
        std::optional<ui32> compute_index;
        
        VkQueue graphics;
        VkQueue present;
        VkQueue compute;
        
        bool all() { return (graphics_index.has_value() and present_index.has_value() and
                             compute_index.has_value()); };
    };

    struct SwapchainSupportData {
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
