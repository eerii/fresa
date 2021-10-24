//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_vulkan_core.h"

#include "ftime.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Verse;
using namespace Graphics;

void Vulkan::createUniformBuffers() {
    VkDeviceSize buffer_size = sizeof(VK::UniformBufferObject);
    
    uniform_buffers.resize(swapchain_images.size());
    uniform_buffers_memory.resize(swapchain_images.size());
    
    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    for (int i = 0; i < swapchain_images.size(); i++)
        createBuffer(buffer_size, usage_flags, memory_flags, uniform_buffers[i], uniform_buffers_memory[i]);
    
    log::graphics("Created Vulkan Uniform Buffers");
}

void Vulkan::createDescriptorPool() {
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = static_cast<ui32>(swapchain_images.size());
    
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;
    create_info.maxSets = static_cast<ui32>(swapchain_images.size());
    
    if (vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool) != VK_SUCCESS)
        log::error("Failed to create a Vulkan Descriptor Pool");
    
    log::graphics("Created a Vulkan Descriptor Pool");
}

void Vulkan::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(swapchain_images.size(), descriptor_set_layout);
    
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = descriptor_pool;
    allocate_info.descriptorSetCount = static_cast<ui32>(swapchain_images.size());
    allocate_info.pSetLayouts = layouts.data();
    
    descriptor_sets.resize(swapchain_images.size());
    if (vkAllocateDescriptorSets(device, &allocate_info, descriptor_sets.data()) != VK_SUCCESS)
        log::error("Failed to allocate Vulkan Descriptor Sets");
    
    log::graphics("Allocated Vulkan Descriptor Sets");
    
    for (int i = 0; i < swapchain_images.size(); i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE; //Also possible to use sizeof(VK::UniformBufferObject);
        
        VkWriteDescriptorSet write_descriptor{};
        write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor.dstSet = descriptor_sets[i];
        write_descriptor.dstBinding = 0; //Binding, specified in the shader
        write_descriptor.dstArrayElement = 0;
        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor.descriptorCount = 1;
        write_descriptor.pBufferInfo = &buffer_info;
        write_descriptor.pImageInfo = nullptr;
        write_descriptor.pTexelBufferView = nullptr;
        
        vkUpdateDescriptorSets(device, 1, &write_descriptor, 0, nullptr);
    }
}

void Vulkan::updateUniformBuffer(ui32 current_image) {
    //EXAMPLE FUNCTION
    
    static Clock::time_point start_time = time();
    float t = sec(time() - start_time);
    
    VK::UniformBufferObject ubo{};
    
    ubo.model = glm::rotate(glm::mat4(1.0f), t * 1.570796f, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, 0.3f * std::sin(t * 1.570796f)));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float) swapchain_extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    
    void* data;
    vkMapMemory(device, uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniform_buffers_memory[current_image]);
}

#endif
