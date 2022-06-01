//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_buffers.h"
#include "r_shaders.h"
#include <set>

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
    using MeshID = ui32;
    
    //: Different types of draw batches
    //      Static objects are not updated every frame, not reuploaded to the GPU
    enum DrawBatchType {
        DRAW_STATIC         =  1 << 0,
        DRAW_DYNAMIC        =  1 << 1,
    };

    //: Transform matrix for each of the rendered instances, stored in the draw_transform buffer
    //      Accessed using both the batch id and the batch offset
    //      Also contains a GPU only buffer that is syncronized every frame with the new transforms
    struct DrawTransformData {
        glm::mat4 model;
    };

    //: Material data
    struct DrawMaterialData {
        glm::vec4 color;
    };
    using MaterialID = ui32;

    //: Instance indices
    //      TODO: WIP
    struct DrawInstanceData {
        ui32 transform_id;
        MaterialID material_id;
    };

    //: Draw description that holds references to everything needed to render an object, indexed by DrawID
    struct DrawDescription {
        ui32 hash;
        DrawBatchType type;
        MeshID mesh;
        MaterialID material;
        ui32 count;
        ui32 offset;
        std::vector<ShaderID> shaders;
    };
    using DrawID = ui32;

    //: Draw queue object
    struct DrawQueueObject {
        ShaderID shader;
        ui32 hash;
        DrawCommandID command;
        MeshID mesh;
        ui32 instance_count;
        ui32 instance_offset;
    };
    
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
    
    //: Transform buffer
    inline BlockBuffer draw_transform;
    inline BufferData draw_transform_gpu;
    inline void* draw_transform_data = nullptr;

    //: Material buffer
    inline BlockBuffer draw_materials;

    //: Instance buffer
    inline BlockBuffer draw_instances;
    
    //: Draw queues
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
    
        //: Register material
        MaterialID registerMaterial(glm::vec4 color);
        
        //: Register draw id
        //      Using some relevant information (mesh, ...) create a DrawID handle and add it to batches
        DrawID registerDrawID(MeshID mesh, MaterialID material, std::vector<ShaderID> shaders, DrawBatchType type, ui32 count = 1);
        
        //: Draw, add a draw id to a shader draw queue
        void draw(DrawID id);
        
        //: Get instance data buffer pointer
        DrawTransformData* getInstanceData(DrawID id);
        
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
