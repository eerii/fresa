//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"
#include "reflection.h"
#include "log.h"
#include "config.h"

#include <optional>
#include <variant>
#include <bitset>

//---------------------------------------------------
//: External libraries (warnings disabled)
//---------------------------------------------------
#ifndef _MSC_VER
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Weverything" //: Disable warnings for external libraries
#endif

    //: GLM, glsl-like linear algebra
    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>

    //: SPIRV-cross, shader reflection and cross compilation
    #include "spirv_glsl.hpp"
    namespace spv_c = spirv_cross;

    //: VMA, memory allocation for Vulkan
    #ifdef USE_VULKAN
        #include "vk_mem_alloc.h"
    #endif

#ifndef _MSC_VER
    #pragma clang diagnostic pop
#endif

//---------------------------------------------------
//: Vulkan
//---------------------------------------------------
#ifdef USE_VULKAN
    //: Vulkan main library
    #include <vulkan/vulkan.h>

    //: SDL layer for Vulkan
    #include <SDL2/SDL_vulkan.h>

    //: Shorthand macros
    #define IF_VULKAN(x) x
    #define IF_OPENGL(x)
#endif

//---------------------------------------------------
//: OpenGL
//---------------------------------------------------
#ifdef USE_OPENGL
    // --- WEB ---
    #ifdef __EMSCRIPTEN__
        
        //: GLES 3 main library
        #include <GLES3/gl3.h>

    // --- MACOS ---
    #elif __APPLE__

        //: Apple has deprecated OpenGL in favor of Metal. Last supported version is 4.1 (no compute shaders).
        //  This line disables all the deprecation warnings so it is still usable. However, the Vulkan version is recommended
        //  when developing for this platform, since thanks to MoltenVK it is cross compiled to Metal on compile.
        #define GL_SILENCE_DEPRECATION

        //: OpenGL main libraries
        #include <OpenGL/OpenGL.h>
        #include <OpenGL/gl3.h>

    // --- LINUX and WINDOWS ---
    #else
    
        //: GLEW, library management for modern OpenGL, it might change to GLAD in the future
        #define USE_GLEW
        #include <GL/glew.h>

        //: OpenGL main library
        #include <GL/gl.h>

        //: SDL layer for OpenGL (not needed for MacOS)
        #include <SDL2/SDL_opengl.h>
        #include <SDL2/SDL_opengl_glext.h>

    #endif

    //: Shorthand macros
    #define IF_VULKAN(x)
    #define IF_OPENGL(x) x
#endif



namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Buffer data
    //      Contains the representation of a buffer for each API
    //---------------------------------------------------
    struct BufferData {
        IF_VULKAN(
            VkBuffer buffer;
            VmaAllocation allocation;
        )
        IF_OPENGL(ui32 buffer;)
    };
    
    enum BufferMemory {
        BUFFER_MEMORY_CPU_ONLY  =  IF_VULKAN(VMA_MEMORY_USAGE_CPU_ONLY)    IF_OPENGL(1 << 0),
        BUFFER_MEMORY_GPU_ONLY  =  IF_VULKAN(VMA_MEMORY_USAGE_GPU_ONLY)    IF_OPENGL(1 << 1),
        BUFFER_MEMORY_BOTH      =  IF_VULKAN(VMA_MEMORY_USAGE_CPU_TO_GPU)  IF_OPENGL(1 << 2),
    };
    using BufferMemoryT = IF_VULKAN(VmaMemoryUsage) IF_OPENGL(BufferMemory);
    
    enum BufferUsage {
        BUFFER_USAGE_UNIFORM       =  IF_VULKAN(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)  IF_OPENGL(1 << 0),
        BUFFER_USAGE_STORAGE       =  IF_VULKAN(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)  IF_OPENGL(1 << 1),
        BUFFER_USAGE_VERTEX        =  IF_VULKAN(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)   IF_OPENGL(1 << 2),
        BUFFER_USAGE_INDEX         =  IF_VULKAN(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)    IF_OPENGL(1 << 3),
        BUFFER_USAGE_TRANSFER_SRC  =  IF_VULKAN(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)    IF_OPENGL(1 << 4),
        BUFFER_USAGE_TRANSFER_DST  =  IF_VULKAN(VK_BUFFER_USAGE_TRANSFER_DST_BIT)    IF_OPENGL(1 << 5),
        BUFFER_USAGE_INDIRECT      =  IF_VULKAN(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) IF_OPENGL(1 << 6),
    };
    using BufferUsageT = IF_VULKAN(VkBufferUsageFlags) IF_OPENGL(BufferUsage);
    
    //---------------------------------------------------
    //: Texture data
    //      Contains the representation of a texture for each API
    //---------------------------------------------------
    struct TextureData {
        int w, h, ch;
        IF_VULKAN(
            VkImage image;
            VkImageView image_view;
            VkFormat format;
            VkImageLayout layout;
            VmaAllocation allocation;
        )
        IF_OPENGL(ui32 image;)
    };
    
    
    //TODO: REFACTOR
    
    //Buffer
    //----------------------------------------

    using GeometryBufferID = ui32;
    struct GeometryBufferData {
        BufferData vertex_buffer;
        BufferData index_buffer;
        ui32 index_size;
        ui8 index_bytes;
        #ifdef USE_OPENGL
        ui32 vao;
        #endif
    };
    
    using InstancedBufferID = ui32;
    inline InstancedBufferID no_instance = 0;
    struct InstancedBufferData {
        BufferData instance_buffer;
        ui32 instance_count;
    };
    
    struct UniformBufferObject {
        Members(UniformBufferObject, model, view, proj)
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    //----------------------------------------

    //Texture
    //----------------------------------------
    using TextureID = ui32;
    inline TextureID no_texture = 0;
    
    enum Channels {
        TEXTURE_CHANNELS_G = 1,
        TEXTURE_CHANNELS_GA = 2,
        TEXTURE_CHANNELS_RGB = 3,
        TEXTURE_CHANNELS_RGBA = 4
    };
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
        ATTACHMENT_MSAA = 1 << 6,
        ATTACHMENT_COLOR_INPUT = ATTACHMENT_COLOR | ATTACHMENT_INPUT,
        ATTACHMENT_DEPTH_INPUT = ATTACHMENT_DEPTH | ATTACHMENT_INPUT,
        ATTACHMENT_COLOR_SWAPCHAIN = ATTACHMENT_COLOR | ATTACHMENT_SWAPCHAIN | ATTACHMENT_WINDOW,
        ATTACHMENT_COLOR_EXTERNAL = ATTACHMENT_COLOR | ATTACHMENT_EXTERNAL,
    };
    
    inline std::map<str, AttachmentType> attachment_type_names = {
        {"color", ATTACHMENT_COLOR},
        {"depth", ATTACHMENT_DEPTH},
        {"input", ATTACHMENT_INPUT},
        {"swapchain", ATTACHMENT_SWAPCHAIN},
        {"window", ATTACHMENT_WINDOW},
        {"external", ATTACHMENT_EXTERNAL},
        {"msaa", ATTACHMENT_MSAA},
    };
    
    struct AttachmentData {
        AttachmentType type;
        Vec2<ui16> size;
        
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
        std::vector<AttachmentID> external_attachments;
        std::map<AttachmentID, AttachmentType> attachment_descriptions;
        std::map<AttachmentID, SubpassID> previous_subpass_dependencies;
        #if defined USE_OPENGL
        ui32 framebuffer;
        bool has_depth;
        #endif
    };
    
    struct RenderPassData {
        #if defined USE_VULKAN
        VkRenderPass render_pass;
        std::vector<VkFramebuffer> framebuffers;
        VkExtent2D attachment_extent;
        #endif
    };
    //----------------------------------------
    
    //Shader
    //----------------------------------------
    using ShaderID = str;
    //----------------------------------------

    //Draw
    //----------------------------------------
    using DrawUniformID = ui32;
    struct DrawUniformData {
        std::vector<std::array<BufferData, Config::frames_in_flight>> uniform_buffers;
        #ifdef USE_VULKAN
        std::vector<ui16> size;
        #endif
    };
    
    using IndirectBufferID = ui16;
    inline IndirectBufferID no_indirect_buffer = 0;
    struct IndirectCommandBuffer {
        BufferData buffer;
        ui32 current_offset;
        std::vector<ui32> free_positions;
    };
    
    struct DrawDescription {
        ShaderID shader;
        TextureID texture;
        DrawUniformID uniform;
        GeometryBufferID geometry;
        InstancedBufferID instance = no_instance;
        IndirectBufferID indirect_buffer = no_indirect_buffer;
        ui32 indirect_offset = 0;
    };
    
    //: This is a hierarchical map for rendering
    //  - Regular rendering
    //      1: Vertex buffer (geometry)                 GeometryBufferData
    //      2: Texture                                  TextureData
    //      3: Uniforms (global + per instance)         DrawUniformData
    //  - Instanced rendedring
    //      1: Global uniforms                          DrawUniformData
    //      2: Vertex buffer (geometry)                 GeometryBufferData
    //      3: Per instance buffer                      InstancedBufferID
    //      .: Textures not supported yet
    //: Indirect rendering is the same as instanced, but it uses a command buffer
    
    using DrawQueueUniform = std::map<DrawUniformID, DrawDescription*>;
    using DrawQueueTexture = std::map<TextureID, DrawQueueUniform>;
    using DrawQueueGeometry = std::map<GeometryBufferID, DrawQueueTexture>;
    using DrawQueue = std::map<ShaderID, DrawQueueGeometry>;
    
    using DrawIQueueInstance = std::map<InstancedBufferID, DrawDescription*>;
    using DrawIQueueGeometry = std::map<GeometryBufferID, DrawIQueueInstance>;
    using DrawIQueueUniform = std::map<DrawUniformID, DrawIQueueGeometry>;
    using DrawIQueue = std::map<ShaderID, DrawIQueueUniform>;
    
    // What do i need for drawing?
    // - Uniform buffers
    //      - On regular rendering these include *per instance* and global attributes (model, view...), inside a descriptor set
    //      - On instanced rendering these only include *global* attibutes
    // - Vertex buffers
    //      - On regular rendering these contain geometry, color, uvs... model properties, these also have an index buffer for optimizations
    //      - On instanced rendering, we have two vertex buffers: the regular one and another one with the per instance attributes
    //        The per instance vertex uses VK_VERTEX_INPUT_RATE_INSTANCE
    // - Textures
    //      - On regular rendering there are either binded (OpenGL) or in a descriptor set along with the uniform buffers (Vulkan)
    //      - Not working with them yet for instancing
    
    //----------------------------------------
    
    //Vertex
    //----------------------------------------
    //Needs to be ordered the same way as the shader
    
    //: You can add additional vertex data definitions creating a file called "vertex_list.h" and including it in your project
    //: An example of such definition is as follows
    //      struct VertexExample {
    //          Members(VertexExample, a, b)
    //          glm::vec3 a;
    //          glm::vec2 b;
    //      };
    //: Please also create a variant with all custom types
    //      using CustomVertexType = std::variant<VertexExample, ...>
    //: To specify it in the renderer_description, the name will be everything except for Vertex, so in this case, it is "example" (a-A is the same)
    
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
    
    struct VertexPos2 {
        Members(VertexPos2, pos);
        glm::vec2 pos;
    };
    
    struct VertexPos2Color {
        Members(VertexPos2Color, pos, color);
        glm::vec2 pos;
        glm::vec3 color;
    };
    
    struct VertexPos2UV {
        Members(VertexPos2UV, pos, uv);
        glm::vec2 pos;
        glm::vec2 uv;
    };
    
    struct VertexPos3 {
        Members(VertexPos3, pos);
        glm::vec3 pos;
    };
    
    struct VertexPos3Color {
        Members(VertexPos3Color, pos, color);
        glm::vec3 pos;
        glm::vec3 color;
    };
    
    struct VertexPos3UV {
        Members(VertexPos3UV, pos, uv);
        glm::vec3 pos;
        glm::vec2 uv;
    };
    
    struct VertexOBJ {
        Members(VertexOBJ, pos, uv, normal);
        glm::vec3 pos;
        glm::vec2 uv;
        glm::vec3 normal;
    };
    
    namespace Vertices {
        inline const std::vector<VertexPos2> rect2 = {
            {{0.f, 0.f}},
            {{1.f, 0.f}},
            {{1.f, 1.f}},
            {{0.f, 1.f}},
        };
        
        inline const std::vector<VertexPos2UV> rect2_tex = {
            {{0.f, 0.f}, {0.0f, 0.0f}},
            {{1.f, 0.f}, {1.0f, 0.0f}},
            {{1.f, 1.f}, {1.0f, 1.0f}},
            {{0.f, 1.f}, {0.0f, 1.0f}},
        };
        
        inline const std::vector<VertexPos2Color> rect2_color = {
            {{0.f, 0.f}, {1.0f, 0.0f, 0.0f}},
            {{1.f, 0.f}, {0.0f, 1.0f, 0.0f}},
            {{1.f, 1.f}, {0.0f, 0.0f, 1.0f}},
            {{0.f, 1.f}, {1.0f, 1.0f, 1.0f}},
        };
        
        inline const std::vector<VertexPos3> rect3 = {
            {{0.f, 0.f, 0.0f}},
            {{1.f, 0.f, 0.0f}},
            {{1.f, 1.f, 0.0f}},
            {{0.f, 1.f, 0.0f}},
        };
        
        inline const std::vector<VertexPos3UV> rect3_tex = {
            {{0.f, 0.f, 0.f}, {0.0f, 0.0f}},
            {{1.f, 0.f, 0.f}, {1.0f, 0.0f}},
            {{1.f, 1.f, 0.f}, {1.0f, 1.0f}},
            {{0.f, 1.f, 0.f}, {0.0f, 1.0f}},
        };
        
        inline const std::vector<VertexPos3Color> rect3_color = {
            {{0.f, 0.f, 0.f}, {1.0f, 0.0f, 0.0f}},
            {{1.f, 0.f, 0.f}, {0.0f, 1.0f, 0.0f}},
            {{1.f, 1.f, 0.f}, {0.0f, 0.0f, 1.0f}},
            {{0.f, 1.f, 0.f}, {1.0f, 1.0f, 1.0f}},
        };
        
        inline const std::vector<VertexPos3> cube = {
            {{-1.f, -1.f, -1.f}},
            {{ 1.f, -1.f, -1.f}},
            {{ 1.f,  1.f, -1.f}},
            {{-1.f,  1.f, -1.f}},
            {{-1.f, -1.f,  1.f}},
            {{ 1.f, -1.f,  1.f}},
            {{ 1.f,  1.f,  1.f}},
            {{-1.f,  1.f,  1.f}},
        };
        
        inline const std::vector<VertexPos3Color> cube_color = {
            {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}}, //Light
            {{ 1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}}, //Teal
            {{ 1.f,  1.f, -1.f}, {1.000f, 0.815f, 0.019f}}, //Yellow
            {{-1.f,  1.f, -1.f}, {0.988f, 0.521f, 0.113f}}, //Orange
            {{-1.f, -1.f,  1.f}, {0.925f, 0.254f, 0.345f}}, //Red
            {{ 1.f, -1.f,  1.f}, {0.925f, 0.235f, 0.647f}}, //Pink
            {{ 1.f,  1.f,  1.f}, {0.658f, 0.180f, 0.898f}}, //Purple
            {{-1.f,  1.f,  1.f}, {0.258f, 0.376f, 0.941f}}, //Blue
        };
        
        inline const std::vector<VertexPos2> window = {
            {{-1.f, -1.f}}, {{-1.f, 1.f}},
            {{ 1.f, -1.f}}, {{ 1.f, 1.f}},
            {{ 1.f, -1.f}}, {{-1.f, 1.f}},
        };
    }
    
    namespace Indices {
        inline const std::vector<ui16> rect = {
            0, 2, 1, 0, 3, 2
        };
        
        inline const std::vector<ui16> cube = {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1,
            5, 4, 6, 6, 4, 7,
        };
    }
    
    using FresaVertexType = std::variant<VertexPos2, VertexPos2Color, VertexPos2UV, VertexPos3, VertexPos3Color, VertexPos3UV, VertexOBJ>;
    
    #if __has_include("vertex_list.h")
        #include "vertex_list.h"
    #else
        using CustomVertexType = std::variant<>;
    #endif
    
    using VertexType = concatenate_<FresaVertexType, CustomVertexType>::type;
    //----------------------------------------
}
