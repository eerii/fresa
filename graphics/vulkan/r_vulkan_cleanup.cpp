//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

using namespace Verse;
using namespace Graphics;

void Vulkan::destroySwapchain() {
    for (VkFramebuffer fb : swapchain_framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);
    
    vkFreeCommandBuffers(device, command_pool, static_cast<ui32>(command_buffers.size()), command_buffers.data());
    
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    
    for (VkImageView view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);
    
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    
    for (int i = 0; i < swapchain_images.size(); i++) {
        vkDestroyBuffer(device, uniform_buffers[i], nullptr);
        vkFreeMemory(device, uniform_buffers_memory[i], nullptr);
    }
    
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
}

void Vulkan::destroy() {
    vkDeviceWaitIdle(device);
    
    destroySwapchain();
    
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    
    vkDestroyBuffer(device, vertex_buffer, nullptr);
    vkFreeMemory(device, vertex_buffer_memory, nullptr);
    
    vkDestroyBuffer(device, index_buffer, nullptr);
    vkFreeMemory(device, index_buffer_memory, nullptr);
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, semaphores_image_available[i], nullptr);
        vkDestroySemaphore(device, semaphores_render_finished[i], nullptr);
        vkDestroyFence(device, fences_in_flight[i], nullptr);
    }
    
    vkDestroyCommandPool(device, command_pool, nullptr);
    vkDestroyCommandPool(device, temp_command_pool, nullptr);
    
    vkDestroyDevice(device, nullptr);
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

#endif
