//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "reflection.h"
#include "log.h"

#include <optional>
#include <bitset>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "spirv_glsl.hpp" //SPIRV-cross for reflection

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#endif

#define DRAW_SHADER_MAX 32

namespace Fresa::Graphics
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
        #ifdef USE_OPENGL
        ui32 vao;
        #endif
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

    inline const TextureData no_texture{};
    //----------------------------------------

    //Framebuffer
    //----------------------------------------
    enum AttachmentType {
        ATTACHMENT_COLOR = 1 << 0,
        ATTACHMENT_DEPTH = 1 << 1,
        ATTACHMENT_INPUT = 1 << 2,
        ATTACHMENT_COLOR_INPUT = ATTACHMENT_COLOR | ATTACHMENT_INPUT,
        ATTACHMENT_DEPTH_INPUT = ATTACHMENT_DEPTH | ATTACHMENT_INPUT,
    };

    struct FramebufferData {
        #ifdef USE_OPENGL
        ui32 id_;
        #endif
    };
    //----------------------------------------

    //Shader
    //----------------------------------------
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;

    enum DrawShaders {
        SHADER_DRAW = 0,
        SHADER_DRAW_2 = 1,
    };

    enum PostShaders {
        SHADER_POST = 0 + DRAW_SHADER_MAX,
    };

    inline std::map<DrawShaders, str> draw_shader_names = {
        {SHADER_DRAW, "test"},
        {SHADER_DRAW_2, "test"},
    };

    inline std::map<PostShaders, str> post_shader_names = {
        
    };

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
        std::optional<TextureID> texture_id;
        DrawShaders shader;
        std::vector<BufferData> uniform_buffers;
        #ifdef USE_VULKAN
        std::vector<VkDescriptorSet> descriptor_sets;
        #endif
    };
    
    //: This is a hierarchical map for rendering
    //  - Geometry data (vao, vertex buffer and index buffer)
    //  - Textures
    //  - Uniforms
    //: I know it looks awfully convoluted, but it makes sense 
    using DrawQueueData = std::pair<const DrawData*, glm::mat4>;
    using DrawQueueMapTextures = std::map<const TextureData*, std::vector<DrawQueueData>>;
    using DrawQueueMapBuffers = std::map<const DrawBufferData*, DrawQueueMapTextures>;
    using DrawQueueMap = std::map<DrawShaders, DrawQueueMapBuffers>;

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
    //----------------------------------------
}