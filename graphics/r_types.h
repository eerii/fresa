//: fresa by jose pazos perez, licensed under GPLv3
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
    //: Textures (TODO: Move to its own module, r_textures)
    //---------------------------------------------------

    //: Texture data
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
    using TextureID = ui32;
    inline TextureID no_texture = 0;

    //: Texture channels
    enum TextureChannels {
        TEXTURE_CHANNELS_G = 1,
        TEXTURE_CHANNELS_GA = 2,
        TEXTURE_CHANNELS_RGB = 3,
        TEXTURE_CHANNELS_RGBA = 4
    };

    //: Attachment types
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

    //: Attachments (TODO: Change from separate components to have a texture data + extra componets)
    struct AttachmentData {
        AttachmentType type;
        Vec2<ui16> size;
        
        IF_VULKAN(
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
        )
        IF_OPENGL(ui32 image;)
    };
    using AttachmentID = ui8;

    //----------------------------------------
    //: Subpasses (TODO: Check where to move this)
    //----------------------------------------
    using SubpassID = ui8;
    using RenderPassID = ui8;
    
    //: Subpass
    struct SubpassData {
        std::vector<AttachmentID> external_attachments;
        std::map<AttachmentID, AttachmentType> attachment_descriptions;
        std::map<AttachmentID, SubpassID> previous_subpass_dependencies;
        IF_OPENGL(
            ui32 framebuffer;
            bool has_depth;
        )
    };
    
    //: Renderpass
    struct RenderPassData {
        IF_VULKAN(
            VkRenderPass render_pass;
            std::vector<VkFramebuffer> framebuffers;
            VkExtent2D attachment_extent;
        )
    };
    
    //----------------------------------------
    //: Vertex (TODO: Check where to move this)
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
    
    //: Vertex formats
    enum VertexFormat {
        VERTEX_FORMAT_R_F = 1,
        VERTEX_FORMAT_RG_F = 2,
        VERTEX_FORMAT_RGB_F = 3,
        VERTEX_FORMAT_RGBA_F = 4,
    };
    
    //: Vertex attribute descriptions
    struct VertexAttributeDescription {
        ui32 binding;
        ui32 location;
        VertexFormat format;
        ui32 offset;
    };
    
    //: Vertex description for OBJ-like objects
    struct VertexOBJ {
        Members(VertexOBJ, pos, uv, normal);
        glm::vec3 pos;
        glm::vec2 uv;
        glm::vec3 normal;
    };
    
    //: Vertex description for rect-like objects in 2 dimensions
    struct VertexPos2 {
        Members(VertexPos2, pos);
        glm::vec2 pos;
    };
    
    //: Vertices of a window rect with propper scaling
    inline const std::vector<VertexPos2> window_vertices = {
        {{-1.f, -1.f}}, {{-1.f, 1.f}},
        {{ 1.f, -1.f}}, {{ 1.f, 1.f}},
        {{ 1.f, -1.f}}, {{-1.f, 1.f}},
    };
    
    //: Vertex type variant, including custom vertex objects
    using FresaVertexType = std::variant<VertexPos2, VertexOBJ>;
    
    #if __has_include("vertex_list.h")
        #include "vertex_list.h"
    #else
        using CustomVertexType = std::variant<>;
    #endif
    
    using VertexType = concatenate_<FresaVertexType, CustomVertexType>::type;
}
