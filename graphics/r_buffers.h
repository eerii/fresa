//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: TODO: MOVE BUFFER DATA HERE
    
    //: Indexing IDs
    using MeshID = FresaType<ui16, struct MeshTag>;
    using UniformBufferID = FresaType<ui16, struct UniformTag>;
    using StorageBufferID = FresaType<ui16, struct StorageTag>;
    
    //: Paddings of each mesh for each of the big vertex and index buffers
    struct MeshPadding {
        ui32 vertex_offset;
        ui32 vertex_size;
        ui32 index_offset;
        ui32 index_size;
    };
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Mesh buffer
    //      Contains two GPU buffers, one for all vertices and one for all indices
    //      They are indexed by using a MeshID that returns a padding for both
    inline struct MeshBuffers {
        BufferData vertex_buffer;
        BufferData index_buffer;
        std::map<MeshID, MeshPadding> paddings;
        ui32 pool_size{0};
    } meshes;
    
    //: Uniform buffers
    inline std::map<UniformBufferID, BufferData> uniform_buffers;
    
    //: Storage buffers
    inline std::map<StorageBufferID, BufferData> storage_buffers;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    
    namespace Buffer {
        MeshBuffers allocateMeshBuffer();
    }
    
    //---------------------------------------------------
    //: API dependen systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    namespace Common {
        BufferData allocateBuffer(ui32 size, BufferUsage usage, BufferMemory memory, void* data = nullptr, bool delete_with_program = true);
        void updateBuffer(BufferData &buffer, void* data, ui32 size, ui32 offset = 0);
        void copyBuffer(BufferData &src, BufferData &dst, ui32 size, ui32 offset = 0);
    }
}
