//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include <set>

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Indexing IDs
    using MeshID = FresaType<ui16, struct MeshTag>;
    using UniformBufferID = FresaType<ui16, struct UniformTag>;
    using StorageBufferID = FresaType<ui16, struct StorageTag>;
    
    inline UniformBufferID no_uniform_buffer{USHRT_MAX};
    inline StorageBufferID no_storage_buffer{USHRT_MAX};
    
    inline std::map<StorageBufferID, str> reserved_buffers {
        {StorageBufferID{0}, "InstanceBuffer"},
    };
    
    //: Buffer memory type (CPU, GPU or both)
    enum BufferMemory {
        BUFFER_MEMORY_CPU_ONLY  =  IF_VULKAN(VMA_MEMORY_USAGE_CPU_ONLY)    IF_OPENGL(1 << 0),
        BUFFER_MEMORY_GPU_ONLY  =  IF_VULKAN(VMA_MEMORY_USAGE_GPU_ONLY)    IF_OPENGL(1 << 1),
        BUFFER_MEMORY_BOTH      =  IF_VULKAN(VMA_MEMORY_USAGE_CPU_TO_GPU)  IF_OPENGL(1 << 2),
    };
    using BufferMemoryT = IF_VULKAN(VmaMemoryUsage) IF_OPENGL(BufferMemory);
    
    //: Buffer usage
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
    
    //: Paddings of each mesh for each of the big vertex and index buffers
    struct MeshPadding {
        ui32 vertex_last;   //: Number of the last vertex
        ui32 vertex_offset; //: Offset in bytes of the first vertex
        ui32 vertex_size;   //: Number of vertices in this block
        ui8 vertex_bytes;   //: Bytes of one vertex
        
        ui32 index_last;
        ui32 index_offset;
        ui32 index_size;
        ui8 index_bytes;
    };
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Buffer data
    //      Contains the representation of a buffer for each API
    struct BufferData {
        IF_VULKAN(
            VkBuffer buffer;
            VmaAllocation allocation;
        )
        IF_OPENGL(ui32 buffer;)
        BufferMemory memory;
        bool operator < (const BufferData &b) const { return buffer < b.buffer; }
    };
    
    //: Block buffers
    struct BlockBufferPartition {
        ui32 size; //: In items
        ui32 offset;
    };
    struct BlockBuffer {
        BufferData buffer;
        ui32 stride;
        ui32 allocation_chunk;
        std::vector<BlockBufferPartition> blocks;
        std::vector<BlockBufferPartition> free_blocks;
    };
    
    //: Mesh buffer
    //      Contains two GPU buffers, one for all vertices and one for all indices
    //      They are indexed by using a MeshID that returns a padding for both
    inline struct MeshBuffers {
        BufferData vertex_buffer;
        BufferData index_buffer;
        std::map<MeshID, MeshPadding> paddings;
        ui32 pool_size = 0;
    } meshes;
    
    //: Uniform buffers
    inline std::map<UniformBufferID, BufferData> uniform_buffers; //TODO: One buffer offset per frame in flight
    
    //: Storage buffers
    inline std::map<StorageBufferID, BufferData> storage_buffers;
    
    //: List of all buffers for deletion purposes
    inline std::set<BufferData> buffer_list;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    
    namespace Buffer {
        //: Allocate or grow vertex and index buffers
        MeshBuffers allocateMeshBuffer();
        
        //: Add mesh vertices and indices to the mesh buffers
        MeshID registerMeshInternal(void* vertices, ui32 vertices_size, ui8 vertex_bytes, void* indices, ui32 indices_size, ui8 index_bytes);
        template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
        MeshID registerMesh(const std::vector<V> &vertices, const std::vector<I> &indices) {
            return registerMeshInternal((void*)vertices.data(), (ui32)vertices.size(), (ui8)sizeof(V),
                                        (void*)indices.data(), (ui32)indices.size(), (ui8)sizeof(I));
        }
        
        //: Register uniform buffer
        UniformBufferID registerUniformBuffer(ui32 size);
        
        //: Register storage buffer
        StorageBufferID registerStorageBuffer(str name, ui32 size);
        
        //:
        BlockBuffer createBlockBuffer(ui32 initial_size, ui32 stride, BufferUsage usage, BufferMemory memory);
        
        //:
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    namespace Common {
        BufferData allocateBuffer(ui32 size, BufferUsage usage, BufferMemory memory, void* data = nullptr);
        void updateBuffer(BufferData &buffer, void* data, ui32 size, ui32 offset = 0);
        void copyBuffer(BufferData &src, BufferData &dst, ui32 size, ui32 offset = 0, ui32 src_offset = 0);
        void destroyBuffer(const BufferData &buffer);
    }
}
