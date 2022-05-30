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
    constexpr DrawCommandID no_draw_command = UINT_MAX;
    using IDrawIndexedIndirectCommand = IF_VULKAN(VkDrawIndexedIndirectCommand) IF_OPENGL(GlDrawIndexedIndirectCommand);
    
    //: Mesh id (vertex + index blocks)
    struct MeshID {
        ui32 vertex;
        ui32 index;
    };
    
    //: Draw batch id
    using DrawBatchID = ui32;
    
    //: Different types of draw batches
    //      - Single / Instances: Either render one or a few objects or many
    //        When using instances, each mesh gets its own draw batch, but all the single objects are packed into one batch
    //      - Static / Dynamic: Static objects are not updated every frame, not reuploaded to the GPU
    enum DrawBatchType {
        DRAW_SINGLE_OBJECT  =  1 << 0,
        DRAW_INSTANCES      =  1 << 1,
        DRAW_STATIC         =  1 << 2,
        DRAW_DYNAMIC        =  1 << 3,
    };
    
    //: Draw description that holds references to everything needed to render an object, indexed by DrawID
    struct DrawDescription {
        DrawBatchID batch;
        DrawBatchType type;
        MeshID mesh;
        ui32 count;
        ui32 offset;
    };
    using DrawID = ui32;
    
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
    
    //: Draw description list
    inline std::map<DrawID, DrawDescription> draw_descriptions;
    
    //: Data for each of the rendered instances, stored in the draw_instances buffer
    //      Accessed using both the batch id and the batch offset
    struct DrawInstanceData {
        glm::mat4 model;
    };
    inline BlockBuffer draw_instances;
    inline BufferData draw_instances_gpu;
    inline void* draw_instance_data = nullptr;
    
    //: Draw queue
    struct DrawQueueObject {
        ShaderID shader;
        DrawBatchID batch;
        DrawCommandID command;
        MeshID mesh;
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
        DrawID registerDrawID(MeshID mesh, DrawBatchType type, ui32 count = 1);
        
        //: Draw, add a draw id to a shader draw queue
        void draw(ShaderID shader, DrawID id);
        
        //: Get instance data buffer pointer
        DrawInstanceData* getInstanceData(DrawID id);
        
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
