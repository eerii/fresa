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
    using UniformBufferID = ui16;
    using StorageBufferID = ui16;
    inline ui16 no_buffer_id{USHRT_MAX};
    
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
    
    //: Buffer is not created or invalid
    constexpr auto no_buffer = IF_VULKAN(VK_NULL_HANDLE) IF_OPENGL(0);
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Buffer data
    //      Contains the representation of a buffer for each API
    struct BufferData {
        IF_VULKAN(
            VkBuffer buffer = no_buffer;
            VmaAllocation allocation;
        )
        IF_OPENGL(ui32 buffer;)
        BufferMemory memory;
        bool operator < (const BufferData &b) const { return buffer < b.buffer; }
    };

    //: Reserved buffers
    //      Buffers defined with this name in shaders will not be a part of the bufferlists and instead have a predefined location
    //      They need to be initialized with linkReservedBuffers()
    struct ReservedBuffer {
        ui32 id;
        str name;
        const BufferData* buffer;
    };
    inline std::vector<ReservedBuffer> reserved_buffers {
        {0, "TransformBuffer", nullptr},
        {1, "MaterialBuffer", nullptr},
        {2, "InstanceBuffer", nullptr},
    };
    
    //: Block buffers
    struct BlockBufferPartition {
        ui32 size;
        ui32 offset;
        std::vector<ui32> free_positions;
    };
    struct BlockBuffer {
        BufferData buffer;
        BufferUsage usage;
        ui32 stride;
        ui32 allocation_chunk;
        std::map<ui32, BlockBufferPartition> blocks;
        std::vector<BlockBufferPartition> free_blocks;
        std::function<void(BufferData &buffer)> callback;
    };
    
    //: Uniform buffers
    inline std::map<UniformBufferID, BufferData> uniform_buffers; //TODO: One buffer offset per frame in flight
    
    //: Storage buffers
    inline std::map<StorageBufferID, BufferData> storage_buffers;
    
    //: List of all buffers for deletion purposes
    inline std::set<BufferData> buffer_list;
    
    namespace Buffer {
        //---------------------------------------------------
        //: Systems
        //---------------------------------------------------
        
        //: Register uniform buffer
        UniformBufferID registerUniformBuffer(str name, ui32 size);
        
        //: Register storage buffer
        StorageBufferID registerStorageBuffer(str name, ui32 size);
    
        //:
        void linkReservedBuffer(str name, const BufferData* buffer);
        
        //:
        BlockBuffer createBlockBuffer(ui32 initial_size, ui32 stride, BufferUsage usage, BufferMemory memory,
                                      std::function<void(BufferData &buffer)> callback = [](BufferData &b){});
        
        //:
        ui32 addToBlockBuffer(BlockBuffer &buffer, ui32 block, void* data = nullptr, ui32 count = 1, bool exact = false);
    
        //:
        void removeFromBlockBuffer(BlockBuffer &buffer, ui32 block, ui32 index);
        
        //:
        void growBlockBuffer(BlockBuffer &buffer, ui32 block, ui32 size = 0, bool exact = false);
        
        //---------------------------------------------------
        //: API dependent systems
        //      They are not implemented in this file, instead you can find them in each API code
        //---------------------------------------------------
        namespace API {
            BufferData allocateBuffer(ui32 size, BufferUsage usage, BufferMemory memory, void* data = nullptr);
            void updateBuffer(BufferData &buffer, void* data, ui32 size, ui32 offset = 0);
            void copyBuffer(BufferData &src, BufferData &dst, ui32 size, ui32 offset = 0, ui32 src_offset = 0);
            void destroyBuffer(const BufferData &buffer);
        }
    }
}
