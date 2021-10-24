//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"
#include "r_shader.h"

using namespace Verse;
using namespace Graphics;

void Vulkan::createRenderPass() {
    VkAttachmentDescription color_attachment{};
    
    color_attachment.format = swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    
    VkSubpassDescription subpass = createRenderSubpass();
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    if (vkCreateRenderPass(device, &create_info, nullptr, &render_pass) != VK_SUCCESS)
        log::error("Error creating a Vulkan Render Pass");
    
    log::graphics("Created all Vulkan Render Passes");
}

VkSubpassDescription Vulkan::createRenderSubpass() {
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    
    return subpass;
}

void Vulkan::prepareRenderInfo() {
    prepareRenderInfoVertexInput();
    prepareRenderInfoInputAssembly();
    prepareRenderInfoViewportState();
    prepareRenderInfoRasterizer();
    prepareRenderInfoMultisampling();
    prepareRenderInfoDepthStencil();
    prepareRenderInfoColorBlendAttachment();
    prepareRenderInfoColorBlendState();
}

void Vulkan::prepareRenderInfoVertexInput() {
    rendering_create_info.vertex_input = {};
    rendering_create_info.vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    rendering_create_info.vertex_input_binding_description = Vertex::getBindingDescription();
    rendering_create_info.vertex_input_attribute_descriptions = Vertex::getAttributeDescriptions();
    
    rendering_create_info.vertex_input.vertexBindingDescriptionCount = 1;
    rendering_create_info.vertex_input.pVertexBindingDescriptions = &rendering_create_info.vertex_input_binding_description;
    rendering_create_info.vertex_input.vertexAttributeDescriptionCount = static_cast<ui32>(rendering_create_info.vertex_input_attribute_descriptions.size());
    rendering_create_info.vertex_input.pVertexAttributeDescriptions = rendering_create_info.vertex_input_attribute_descriptions.data();
}

void Vulkan::prepareRenderInfoInputAssembly() {
    rendering_create_info.input_assembly = {};
    
    rendering_create_info.input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    rendering_create_info.input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    rendering_create_info.input_assembly.primitiveRestartEnable = VK_FALSE; //Change when using an index buffer
}

void Vulkan::prepareRenderInfoViewportState() {
    rendering_create_info.viewport = {};
    rendering_create_info.viewport.x = 0.0f;
    rendering_create_info.viewport.y = 0.0f;
    rendering_create_info.viewport.width = (float)swapchain_extent.width;
    rendering_create_info.viewport.height = (float)swapchain_extent.height;
    rendering_create_info.viewport.minDepth = 0.0f;
    rendering_create_info.viewport.maxDepth = 1.0f;
    
    rendering_create_info.scissor = {};
    rendering_create_info.scissor.offset = {0, 0};
    rendering_create_info.scissor.extent = swapchain_extent;
    
    rendering_create_info.viewport_state = {};
    
    rendering_create_info.viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    rendering_create_info.viewport_state.viewportCount = 1;
    rendering_create_info.viewport_state.pViewports = &rendering_create_info.viewport;
    rendering_create_info.viewport_state.scissorCount = 1;
    rendering_create_info.viewport_state.pScissors = &rendering_create_info.scissor;
}

void Vulkan::prepareRenderInfoRasterizer() {
    rendering_create_info.rasterizer = {};
    
    rendering_create_info.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rendering_create_info.rasterizer.depthClampEnable = VK_FALSE;
    rendering_create_info.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    
    rendering_create_info.rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Fill, Line, Point, requires enabling GPU features
    rendering_create_info.rasterizer.lineWidth = 1.0f; //Larger thickness requires enabling GPU features
    
    rendering_create_info.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rendering_create_info.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    rendering_create_info.rasterizer.depthBiasEnable = VK_FALSE;
    rendering_create_info.rasterizer.depthBiasConstantFactor = 0.0f;
    rendering_create_info.rasterizer.depthBiasClamp = 0.0f;
    rendering_create_info.rasterizer.depthBiasSlopeFactor = 0.0f;
}

void Vulkan::prepareRenderInfoMultisampling() {
    rendering_create_info.multisampling = {};
    
    rendering_create_info.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    rendering_create_info.multisampling.sampleShadingEnable = VK_FALSE; //Needs to be enabled in the future
    
    rendering_create_info.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    rendering_create_info.multisampling.minSampleShading = 1.0f;
    rendering_create_info.multisampling.pSampleMask = nullptr;
    
    rendering_create_info.multisampling.alphaToCoverageEnable = VK_FALSE;
    rendering_create_info.multisampling.alphaToOneEnable = VK_FALSE;
}

void Vulkan::prepareRenderInfoDepthStencil() {
    rendering_create_info.depth_stencil = {};
    
    rendering_create_info.depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    rendering_create_info.depth_stencil.depthTestEnable = VK_TRUE;
    rendering_create_info.depth_stencil.depthWriteEnable = VK_TRUE;
    
    rendering_create_info.depth_stencil.depthBoundsTestEnable = VK_FALSE;
    rendering_create_info.depth_stencil.stencilTestEnable = VK_FALSE;
    
    rendering_create_info.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    
    rendering_create_info.depth_stencil.minDepthBounds = 0.0f;
    rendering_create_info.depth_stencil.maxDepthBounds = 1.0f;
    
    rendering_create_info.depth_stencil.front = {};
    rendering_create_info.depth_stencil.back = {};
}

void Vulkan::prepareRenderInfoColorBlendAttachment() {
    rendering_create_info.color_blend_attachment = {};
    
    rendering_create_info.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    rendering_create_info.color_blend_attachment.blendEnable = VK_TRUE;
    
    rendering_create_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    rendering_create_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    rendering_create_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    
    rendering_create_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    rendering_create_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    rendering_create_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void Vulkan::prepareRenderInfoColorBlendState() {
    rendering_create_info.color_blend_state = {};

    rendering_create_info.color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    
    rendering_create_info.color_blend_state.logicOpEnable = VK_FALSE;
    rendering_create_info.color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    
    rendering_create_info.color_blend_state.attachmentCount = 1;
    rendering_create_info.color_blend_state.pAttachments = &rendering_create_info.color_blend_attachment; //Change for all framebuffers
    
    rendering_create_info.color_blend_state.blendConstants[0] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[1] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[2] = 0.0f;
    rendering_create_info.color_blend_state.blendConstants[3] = 0.0f;
}

void Vulkan::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &ubo_layout_binding;
    
    if (vkCreateDescriptorSetLayout(device, &create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS)
        log::error("Error creating the Vulkan Descriptor Set Layout for Uniform Buffers");
}

void Vulkan::createPipelineLayout() {
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &descriptor_set_layout;
    
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        log::error("Error creating the Vulkan Pipeline Layout");
    
    log::graphics("Created the Vulkan Pipeline Layout");
}

void Vulkan::createGraphicsPipeline() {
    std::vector<char> vert_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/test.vert.spv");
    std::vector<char> frag_shader_code = Graphics::Shader::readSPIRV("res/shaders/test/test.frag.spv");
    
    Graphics::Shader::ShaderStages stages;
    stages.vert = Graphics::Shader::createShaderModule(vert_shader_code, device);
    stages.frag = Graphics::Shader::createShaderModule(frag_shader_code, device);
    std::vector<VkPipelineShaderStageCreateInfo> stage_info = Graphics::Shader::createShaderStageInfo(stages);
    
    createPipelineLayout();
    
    prepareRenderInfo();
    
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    create_info.stageCount = (int)stage_info.size();
    create_info.pStages = stage_info.data();
    
    create_info.pVertexInputState = &rendering_create_info.vertex_input;
    create_info.pInputAssemblyState = &rendering_create_info.input_assembly;
    create_info.pViewportState = &rendering_create_info.viewport_state;
    create_info.pRasterizationState = &rendering_create_info.rasterizer;
    create_info.pMultisampleState = &rendering_create_info.multisampling;
    create_info.pDepthStencilState = &rendering_create_info.depth_stencil;
    create_info.pColorBlendState = &rendering_create_info.color_blend_state;
    create_info.pDynamicState = nullptr;
    create_info.pTessellationState = nullptr;
    
    create_info.layout = pipeline_layout;
    
    create_info.renderPass = render_pass;
    create_info.subpass = 0;
    
    create_info.basePipelineHandle = VK_NULL_HANDLE; //It is not a derived pipeline
    create_info.basePipelineIndex = -1;
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS)
        log::error("Error while creating the Vulkan Graphics Pipeline");
    
    log::graphics("Created the Vulkan Graphics Pipeline");
    
    vkDestroyShaderModule(device, stages.vert.value(), nullptr);
    vkDestroyShaderModule(device, stages.frag.value(), nullptr);
}

#endif
