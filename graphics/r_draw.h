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
    using DrawCommandID = ui32;
    
    //: Size of the draw indirect commands
    constexpr ui32 draw_command_size = IF_VULKAN(sizeof(VkDrawIndexedIndirectCommand)) IF_OPENGL(1);
    
    //: Mesh id (vertex + index blocks)
    struct MeshID {
        ui32 vertex;
        ui32 index;
    };
    
    //: Draw object id
    using DrawID = FresaType<ui32, struct DrawTag>;
    
    //: Draw batch id
    using DrawBatchID = FresaType<ui32, struct DrawBatchTag>;
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Buffer holding the indirect draw command information, as well as the offsets for each command id
    inline BlockBuffer draw_commands;
    
    //: Mesh buffer
    //      Contains two GPU buffers, one for all vertices and one for all indices
    //      They are indexed by using a MeshID
    inline struct MeshBuffers {
        BlockBuffer vertex;
        BlockBuffer index;
    } meshes;
    
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
    
    //: Draw queue
    struct DrawQueueObject {
        ShaderID shader;
        DrawBatchID batch;
        MeshID mesh;
        DrawCommandID command;
        ui32 instance_count;
        ui32 instance_offset;
    };
    inline std::vector<DrawQueueObject> draw_queue;
    inline std::vector<DrawQueueObject> previous_draw_queue = {};
    inline std::map<ShaderID, std::vector<DrawCommandID>> built_draw_queue;
    
    namespace Draw {
        //---------------------------------------------------
        //: Systems
        //---------------------------------------------------
        
        //: Register draw command
        DrawCommandID registerDrawCommand(DrawQueueObject draw);
        
        //: Add mesh vertices and indices to the mesh buffers
        MeshID registerMeshInternal(void* vertices, ui32 vertices_size, ui8 vertex_bytes, void* indices, ui32 indices_size, ui8 index_bytes);
        template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
        MeshID registerMesh(const std::vector<V> &vertices, const std::vector<I> &indices) {
            return registerMeshInternal((void*)vertices.data(), (ui32)vertices.size(), (ui8)sizeof(V),
                                        (void*)indices.data(), (ui32)indices.size(), (ui8)sizeof(I));
        }
        
        //: Register draw id
        //      Using some relevant information (mesh, ...) create a DrawID handle and add it to batches
        DrawID registerDrawID(MeshID mesh);
        
        //: Draw, add a draw id to a shader draw queue
        void draw(ShaderID shader, DrawID id, ui32 count = 1);
        
        //: Allocate scene batch, create or expand a batch block
        void allocateSceneBatchBlock(DrawBatchID batch);
        
        //: Get instance data buffer pointer
        DrawInstanceData* getInstanceData(DrawID id, ui32 count = 1);
        
        //: Build draw queue and create the indirect commands
        void buildDrawQueue();
        
        //---------------------------------------------------
        //: API dependent systems
        //      They are not implemented in this file, instead you can find them in each API code
        //---------------------------------------------------
        namespace API {
            
        }
    }
    
    
}
