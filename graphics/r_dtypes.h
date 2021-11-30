//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "reflection.h"
#include "log.h"

#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "spirv_glsl.hpp" //SPIRV-cross for reflection

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#endif

namespace Verse::Graphics
{
    //Window
    //----------------------------------------
    struct WindowData {
        SDL_Window* window;
        Vec2<> size;
        ui16 refresh_rate;
        Vec2<> resolution;
        ui16 scale;
        bool vsync;
    };
    //----------------------------------------

    //Buffer
    //----------------------------------------
    struct BufferData {
        #if defined USE_OPENGL
        ui32 id_;
        #elif defined USE_VULKAN
        VkBuffer buffer;
        VmaAllocation allocation;
        #endif
    };

    using DrawBufferID = ui32;

    struct DrawBufferData {
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_size;
    };
    //----------------------------------------

    //Uniform
    //----------------------------------------
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    //----------------------------------------

    //Texture
    //----------------------------------------
    using TextureID = ui32;

    enum Channels {
        TEXTURE_CHANNELS_G = 1,
        TEXTURE_CHANNELS_GA = 2,
        TEXTURE_CHANNELS_RGB = 3,
        TEXTURE_CHANNELS_RGBA = 4
    };

    struct TextureData {
        int w, h, ch;
    #if defined USE_OPENGL
        ui32 id_;
    #elif defined USE_VULKAN
        VkImage image;
        VmaAllocation allocation;
        VkFormat format;
        VkImageLayout layout;
        VkImageView image_view;
    #endif
    };
    //----------------------------------------

    //Framebuffer
    //----------------------------------------
    enum FramebufferType {
        FRAMEBUFFER_COLOR_ATTACHMENT,
        FRAMEBUFFER_DEPTH_ATTACHMENT,
        FRAMEBUFFER_COLOR_DEPTH_ATTACHMENT
    };

    struct FramebufferData {
        FramebufferType type;
        std::optional<TextureData> color_texture;
        std::optional<TextureData> depth_texture;
        #ifdef USE_OPENGL
        ui32 id_;
        #endif
    };
    //----------------------------------------

    //Shader
    //----------------------------------------
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;

    struct ShaderLocations {
        std::optional<str> vert;
        std::optional<str> frag;
        std::optional<str> compute;
        std::optional<str> geometry;
    };

    struct ShaderCode {
        std::optional<std::vector<char>> vert;
        std::optional<std::vector<char>> frag;
        std::optional<std::vector<char>> compute;
        std::optional<std::vector<char>> geometry;
    };

    #ifdef USE_VULKAN
    struct ShaderStages {
        std::optional<VkShaderModule> vert;
        std::optional<VkShaderModule> frag;
        std::optional<VkShaderModule> compute;
        std::optional<VkShaderModule> geometry;
    };
    #endif

    struct ShaderData {
        ShaderLocations locations;
        ShaderCode code;
    #if defined USE_OPENGL
        ui8 pid;
        std::map<str, ui32> uniforms;
    #elif defined USE_VULKAN
        ShaderStages stages;
    #endif
    };
    //----------------------------------------

    //Draw
    //----------------------------------------
    using DrawID = ui32;

    struct DrawData {
        DrawBufferID buffer_id;
        std::vector<BufferData> uniform_buffers;
        std::optional<TextureID> texture_id;
        #ifdef USE_VULKAN
        std::vector<VkDescriptorSet> descriptor_sets;
        #endif
    };

    struct DrawQueueInfo {
        const DrawBufferData* buffer;
        const DrawData* data;
        glm::mat4 model;
    };
    //----------------------------------------

    //Vertex
    //----------------------------------------
    //Needs to be ordered the same way as the shader
    struct VertexData {
        Serialize(VertexData, pos, color, uv);
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };

    enum VertexFormat {
        VERTEX_FORMAT_R_F = 1,
        VERTEX_FORMAT_RG_F = 2,
        VERTEX_FORMAT_RGB_F = 3,
        VERTEX_FORMAT_RGBA_F = 4,
    };

    struct VertexAttributeDescription {
        ui32 binding;
        ui32 location;
        VertexFormat format;
        ui32 offset;
    };

    #ifdef USE_OPENGL
    struct VertexArrayData {
        ui32 id_;
        std::vector<VertexAttributeDescription> attributes;
    };
    #endif
    //----------------------------------------
}
