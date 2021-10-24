//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

void Vulkan::createFramebuffers() {
    swapchain_framebuffers.resize(swapchain_image_views.size());
    
    for (int i = 0; i < swapchain_framebuffers.size(); i++) {
        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &swapchain_image_views[i];
        create_info.width = swapchain_extent.width;
        create_info.height = swapchain_extent.height;
        create_info.layers = 1;
        
        if (vkCreateFramebuffer(device, &create_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Framebuffer");
    }
    
    log::graphics("Created all Vulkan Framebuffers");
}

void Vulkan::createCommandPools() {
    //Main command pool
    
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_families.graphics_queue_family_index.value(); //This is the command pool for graphics
    create_info.flags = 0;
    
    if (vkCreateCommandPool(device, &create_info, nullptr, &command_pool) != VK_SUCCESS)
        log::error("Failed to create the Vulkan Graphics Command Pool");
    
    //Temporary command pool for short lived objects
    
    VkCommandPoolCreateInfo temp_create_info{};
    temp_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    temp_create_info.queueFamilyIndex = queue_families.graphics_queue_family_index.value();
    temp_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    
    if (vkCreateCommandPool(device, &create_info, nullptr, &temp_command_pool) != VK_SUCCESS)
        log::error("Failed to create the Vulkan Graphics Temporary Command Pool");
    
    log::graphics("Created all Vulkan Command Pools");
}

void Vulkan::createCommandBuffers() {
    command_buffers.resize(swapchain_framebuffers.size());
    
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = (ui32)command_buffers.size();
    
    if (vkAllocateCommandBuffers(device, &allocate_info, command_buffers.data()) != VK_SUCCESS)
        log::error("Failed to allocate a Vulkan Command Buffer");
    
    for (int i = 0; i < command_buffers.size(); i++) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;
        
        if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Command Buffer");
        
        recordCommandBuffer(command_buffers[i], swapchain_framebuffers[i], descriptor_sets[i]);
        
        if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
            log::error("Failed to end recording on a Vulkan Command Buffer");
    }
    
    log::graphics("Created all Vulkan Command Buffers");
}

void Vulkan::recordCommandBuffer(VkCommandBuffer &command_buffer, VkFramebuffer &framebuffer, VkDescriptorSet &descriptor_set) {
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.02f, 0.02f, 0.02f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};
    
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values.data();
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    VkBuffer vertex_buffers[]{ vertex_buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    
    vkCmdDrawIndexed(command_buffer, index_buffer_size, 1, 0, 0, 0);
    
    vkCmdEndRenderPass(command_buffer);
}

void Vulkan::createSyncObjects() {
    semaphores_image_available.resize(MAX_FRAMES_IN_FLIGHT);
    semaphores_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
    fences_in_flight.resize(MAX_FRAMES_IN_FLIGHT);
    fences_images_in_flight.resize(swapchain_images.size(), VK_NULL_HANDLE);
    
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphores_image_available[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Semaphore");
        
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphores_render_finished[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Semaphore");
        
        if (vkCreateFence(device, &fence_info, nullptr, &fences_in_flight[i]) != VK_SUCCESS)
            log::error("Failed to create a Vulkan Fence");
    }
    
    log::graphics("Created all Vulkan Semaphores");
}

void Vulkan::renderFrame(Config &c) {
    vkWaitForFences(device, 1, &fences_in_flight[current_frame], VK_TRUE, UINT64_MAX);
    
    VkResult result;
    ui32 image_index;
    result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphores_image_available[current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(c);
        return;
    }
    if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR) {
        log::error("Failed to acquire Swapchain Image");
    }
    
    
    if (fences_images_in_flight[image_index] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &fences_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    fences_images_in_flight[image_index] = fences_in_flight[current_frame];
    
    VkSemaphore wait_semaphores[]{ semaphores_image_available[current_frame] };
    VkPipelineStageFlags wait_stages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[]{ semaphores_render_finished[current_frame] };
    
    
    updateUniformBuffer(image_index);
    
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[image_index];
    
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    
    vkResetFences(device, 1, &fences_in_flight[current_frame]);
    
    if (vkQueueSubmit(graphics_queue, 1, &submit_info, fences_in_flight[current_frame]) != VK_SUCCESS)
        log::error("Failed to submit Draw Command Buffer");
    
    
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[]{ swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    
    
    result = vkQueuePresentKHR(present_queue, &present_info);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR)
        recreateSwapchain(c);
    else if (result != VK_SUCCESS)
        log::error("Failed to present Swapchain Image");
    
    
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

#endif
