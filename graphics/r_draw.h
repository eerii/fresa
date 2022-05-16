//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_buffers.h"
#include "r_shaders.h"

namespace Fresa::Graphics
{
    //TODO: IMPORTANT, DELETE OLD BUFFERS, HERE AND IN R_BUFFERS!!!
    
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Indirect draw command buffer id
    using DrawCommandID = FresaType<ui16, struct DrawCommandTag>;
    constexpr DrawCommandID no_draw_buffer{USHRT_MAX};
    
    //: Size of the draw indirect commands
    constexpr ui32 draw_command_size = IF_VULKAN(sizeof(VkDrawIndexedIndirectCommand)) IF_OPENGL(1);
    
    //: Draw object id
    using DrawID = FresaType<ui32, struct DrawTag>;
    
    //: Draw batch id
    using DrawBatchID = FresaType<ui32, struct DrawBatchTag>;
    
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
    constexpr ui32 no_draw_batch_offset = UINT_MAX;
    struct DrawDescription {
        DrawBatchID batch;
        ui32 batch_offset = no_draw_batch_offset;
        MeshID mesh;
        glm::mat4 transform;
    };
    
    //:
    struct DrawBatchBlock {
        ui32 offset;
        ui32 size;
        std::vector<ui32> free_positions = { 0 };
    };
    inline struct DrawScene {
        BufferData buffer;
        std::map<DrawID, DrawDescription> objects;
        std::map<DrawID, DrawDescription> recreate_objects;
        std::map<DrawBatchID, DrawBatchBlock> batches;
        std::vector<DrawBatchBlock> free_blocks = { DrawBatchBlock{} };
        ui32 stride; //
    } draw_scene;
    
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
        DrawID registerDrawID(MeshID mesh, glm::mat4 initial_transform);
        
        //:
        void draw(DrawID id, ShaderID shader, glm::mat4 transform);
        
        //:
        void allocateSceneBatchBlock(DrawBatchID batch);
        
        //:
        void compileSceneBatches();
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        
    }
}
