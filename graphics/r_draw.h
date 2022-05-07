//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_buffers.h"
#include "r_shaders.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Indirect draw command buffer id
    using DrawCommandID = FresaType<ui16, struct DrawCommandTag>;
    inline DrawCommandID no_draw_buffer{USHRT_MAX};
    
    //: Size of the draw indirect commands
    constexpr ui32 draw_command_size = IF_VULKAN(sizeof(VkDrawIndexedIndirectCommand)) IF_OPENGL(1);
    
    //: Draw object id
    using DrawID = FresaType<ui32, struct DrawTag>;
    
    //: Draw batch id
    using DrawBatchID = FresaType<ui16, struct DrawBatchTag>;
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Buffer holding the indirect draw commands and the offset information
    inline struct DrawCommandBuffer {
        BufferData buffer;
        std::vector<DrawCommandID> free_positions = { DrawCommandID{0} };
        ui32 pool_size = 0;
    } draw_commands;
    
    //:
    struct DrawObjectBlock {
        ui32 offset;
        ui32 size;
        std::vector<ui32> free_positions = { 0 };
    };
    inline struct DrawObjectBuffer {
        BufferData buffer;
        std::map<DrawBatchID, DrawObjectBlock> batch_offsets;
        std::vector<DrawObjectBlock> free_blocks = { DrawObjectBlock{0, 0, {0}} };
        ui32 stride;
    } draw_objects;
    
    //:
    struct DrawDescription {
        ShaderID shader;
        MeshID mesh;
        glm::mat4 transform;
    };
    
    //:
    struct DrawBatch {
        MeshID mesh;
        ui32 instance_offset;
        ui32 instance_count;
    };
    
    //:
    struct DrawScene {
        std::map<DrawID, DrawDescription> objects;
        std::map<DrawID, DrawDescription> recreate_objects;
        std::map<DrawBatchID, DrawBatch> batches;
        // MOVE DRAW OBJECT BUFFER HERE
    };
    
    //:
    inline std::map<ShaderID, DrawScene> draw_scenes;
    
    //TODO: TEMP
    struct TempDrawObject {
        MeshID mesh;
        DrawCommandID indirect;
    };
    inline std::map<ShaderID, TempDrawObject> temp_draw_queue;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    
    namespace Draw {
        //: Allocate draw command buffer
        DrawCommandBuffer allocateDrawCommandBuffer();
        
        //: Register draw command
        DrawCommandID registerDrawCommand(MeshID mesh);
        
        //: Remove draw command
        void removeDrawCommand(DrawCommandID id);
        
        //:
        DrawObjectBuffer allocateDrawObjectBuffer(std::vector<DrawBatchID> batches, ui32 object_size = 0);

        //:
        DrawID registerDrawID(ShaderID shader, MeshID mesh, glm::mat4 initial_transform);
        
        //:
        void draw(DrawID id, glm::mat4 transform);
        
        //:
        void compileSceneBatches(DrawScene &scene);
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        
    }
}
