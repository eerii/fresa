//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"
#include "reflection.h"
#include "log.h"

#include <optional>
#include <variant>
#include <bitset>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "spirv_glsl.hpp" //SPIRV-cross for reflection

#ifdef USE_VULKAN
    #include <vulkan/vulkan.h>
    #include "vk_mem_alloc.h"
#endif

//---Data types for graphics---

namespace Fresa::Graphics
{
    //Window
    //----------------------------------------
    struct WindowData {
        SDL_Window* window;
        
        Vec2<> size;
        ui16 scale;
        ui16 refresh_rate = 0;
        
        bool vsync;
    };
    
    struct CameraData {
        glm::vec3 pos;
        glm::mat4 view;
        glm::mat4 proj;
    };
    
    #if defined USE_VULKAN
    constexpr float viewport_y = -1.0f;
    #elif defined USE_OPENGL
    constexpr float viewport_y = 1.0f;
    #endif
    //----------------------------------------

    //Buffer
    //----------------------------------------
    struct BufferData {
        #if defined USE_VULKAN
        VkBuffer buffer;
        VmaAllocation allocation;
        #elif defined USE_OPENGL
        ui32 id_;
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
        #if defined USE_VULKAN
        VkImage image;
        VmaAllocation allocation;
        VkFormat format;
        VkImageLayout layout;
        VkImageView image_view;
        #elif defined USE_OPENGL
        ui32 id_;
        #endif
    };

    inline const TextureData no_texture{};
    //----------------------------------------

    //Attachments
    //----------------------------------------
    using AttachmentID = ui8;

    enum AttachmentType {
        ATTACHMENT_COLOR = 1 << 0,
        ATTACHMENT_DEPTH = 1 << 1,
        ATTACHMENT_INPUT = 1 << 2,
        ATTACHMENT_SWAPCHAIN = 1 << 3,
        ATTACHMENT_WINDOW = 1 << 4,
        ATTACHMENT_EXTERNAL = 1 << 5,
        ATTACHMENT_COLOR_INPUT = ATTACHMENT_COLOR | ATTACHMENT_INPUT,
        ATTACHMENT_DEPTH_INPUT = ATTACHMENT_DEPTH | ATTACHMENT_INPUT,
        ATTACHMENT_COLOR_SWAPCHAIN = ATTACHMENT_COLOR | ATTACHMENT_SWAPCHAIN | ATTACHMENT_WINDOW,
        ATTACHMENT_COLOR_EXTERNAL = ATTACHMENT_COLOR | ATTACHMENT_EXTERNAL,
    };
    
    struct AttachmentData {
        AttachmentType type;
        Vec2<> size;
        
        #if defined USE_VULKAN
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
        #elif defined USE_OPENGL
        ui32 tex;
        #endif
    };
    //----------------------------------------
    
    //Subpasses
    //----------------------------------------
    using SubpassID = ui8;
    using RenderPassID = ui8;
    
    struct SubpassData {
        std::vector<AttachmentID> attachment_bindings;
        #if defined USE_VULKAN
        std::vector<VkAttachmentReference> color_attachments;
        std::vector<VkAttachmentReference> depth_attachments;
        std::vector<VkAttachmentReference> input_attachments;
        std::vector<AttachmentID> external_attachments;
        std::map<AttachmentID, SubpassID> previous_subpass_dependencies;
        #elif defined USE_OPENGL
        ui32 framebuffer;
        bool has_depth;
        std::vector<AttachmentID> framebuffer_attachments;
        std::vector<AttachmentID> input_attachments;
        #endif
    };
    //----------------------------------------
    
    //Shader
    //----------------------------------------
    using ShaderCompiler = spirv_cross::CompilerGLSL;
    using ShaderResources = spirv_cross::ShaderResources;

    enum Shaders {
        SHADER_DRAW_COLOR,
        SHADER_DRAW_TEX,
        SHADER_POST,
        SHADER_WINDOW,
    };
    #define LAST_DRAW_SHADER SHADER_DRAW_TEX

    inline std::map<Shaders, str> shader_names = {
        {SHADER_DRAW_COLOR, "test_color"},
        {SHADER_DRAW_TEX, "test_tex"},
        {SHADER_POST, "test_subpass"},
        {SHADER_WINDOW, "test_window"}
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
        #if defined USE_VULKAN
        ShaderStages stages;
        #elif defined USE_OPENGL
        ui8 pid;
        std::map<str, ui32> uniforms;
        SubpassID subpass;
        #endif
    };
    //----------------------------------------

    //Draw
    //----------------------------------------
    using DrawID = ui32;

    struct DrawData {
        DrawBufferID buffer_id;
        std::optional<TextureID> texture_id;
        Shaders shader;
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
    using DrawQueueMap = std::map<Shaders, DrawQueueMapBuffers>;

    //----------------------------------------
    
    //Vertex
    //----------------------------------------
    //Needs to be ordered the same way as the shader
    struct VertexDataColor {
        Serialize(VertexDataColor, pos, color);
        glm::vec3 pos;
        glm::vec3 color;
    };
    
    struct VertexDataTexture {
        Serialize(VertexDataTexture, pos, uv);
        glm::vec3 pos;
        glm::vec2 uv;
    };

    struct VertexDataWindow {
        Serialize(VertexDataWindow, pos);
        glm::vec2 pos;
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
