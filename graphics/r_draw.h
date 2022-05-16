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
    
    //: Buffer holding the indirect draw command information, as well as the offsets for each command id
    inline struct DrawCommandBuffer {
        BufferData buffer;
        std::vector<DrawCommandID> free_positions = { DrawCommandID{0} };
        ui32 pool_size = 0;
    } draw_commands;
    
    //: Draw description that holds references to everything needed to render an object
    //      Indexed by DrawID, is the main identifier of each renderable item
    //      DrawBatchID is a hash of different properties and values that indexes a part of the DrawScene buffer
    struct DrawDescription {
        DrawBatchID batch;
        ui32 batch_offset;
        MeshID mesh;
    };
    
    //: Data for each of the rendered instances, stored in the DrawScene buffer
    //      Accessed using both the batch id and the batch offset
    struct DrawInstanceData {
        glm::mat4 model;
    };
    
    //: The draw scene holds all the objects that are rendered
    //      The big buffer is divided into batch blocks, that are indexed with a DrawBatchID
    //      These are bundled together for objects of the same mesh, so they can be rendered with just one draw indirect command
    //      All draw descriptions are stored here as well, used when drawing
    //: TODO: Clean this, rename to DrawBatchBuffer, and make allocateSceneBatchBlock return something (maybe?)
    struct DrawBatchBlock {
        ui32 offset;
        ui32 size;
        std::vector<ui32> free_positions = { 0 };
    };
    inline struct DrawScene {
        BufferData buffer;
        void* buffer_data = nullptr;
        std::map<DrawID, DrawDescription> objects;
        std::map<DrawBatchID, DrawBatchBlock> batches;
        std::vector<DrawBatchBlock> free_blocks = { DrawBatchBlock{} };
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
        //      Creates or expands the draw command buffer by one chunk
        DrawCommandBuffer allocateDrawCommandBuffer();
        
        //: Register draw command
        //      TODO: WIP
        DrawCommandID registerDrawCommand(MeshID mesh);
        
        //: Remove draw command
        void removeDrawCommand(DrawCommandID id);
        
        //: Register draw id
        //      Using some relevant information (mesh, ...) create a DrawID handle and add it to batches
        DrawID registerDrawID(MeshID mesh);
        
        //: Draw, add a draw id to a shader draw queue
        void draw(ShaderID shader, DrawID id);
        //void draw_multiple(ShaderID shader, DrawID first, ui32 count);
        
        //: Allocate scene batch, create or expand a batch block
        void allocateSceneBatchBlock(DrawBatchID batch);
        
        //: Get instance data buffer pointer
        DrawInstanceData* getInstanceData(DrawID id, ui32 count = 1);
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        
    }
}
