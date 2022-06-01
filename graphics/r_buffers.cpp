//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_buffers.h"

using namespace Fresa;
using namespace Graphics;

//---------------------------------------------------
//: Register uniform buffer
//---------------------------------------------------
UniformBufferID Buffer::registerUniformBuffer(str name, ui32 size) {
    //: Find if the buffer is reserved
    for (auto &r : reserved_buffers) {
        if (r.name == name)
            return r.id;
    }
    
    //: Find new id
    UniformBufferID id = uniform_buffers.size() == 0 ? UniformBufferID{} : uniform_buffers.end()->first;
    do { id++; } while (id == no_buffer_id or uniform_buffers.count(id) or
                        std::find_if(reserved_buffers.begin(), reserved_buffers.end(), [&](auto &r){ return r.id == id; }) != reserved_buffers.end());
    
    //: Allocate new buffer
    uniform_buffers[id] = Buffer::API::allocateBuffer(size, BUFFER_USAGE_UNIFORM, BUFFER_MEMORY_BOTH);
    
    return id;
}

//---------------------------------------------------
//: Register storage buffer
//      Adds a storage buffer to the list and returns an id for indexing
//      If the buffer name is part of the key list and the buffer is allocated, return the id
//---------------------------------------------------
StorageBufferID Buffer::registerStorageBuffer(str name, ui32 size) {
    //: Find if the buffer is reserved
    for (auto &r : reserved_buffers) {
        if (r.name == name)
            return r.id;
    }
    
    //: Find new id
    StorageBufferID id = storage_buffers.size() == 0 ? StorageBufferID{} : storage_buffers.end()->first;
    do { id++; } while (id == no_buffer_id or storage_buffers.count(id) or
                        std::find_if(reserved_buffers.begin(), reserved_buffers.end(), [&](auto &r){ return r.id == id; }) != reserved_buffers.end());
    
    //: Allocate new buffer
    storage_buffers[id] = Buffer::API::allocateBuffer(size, BUFFER_USAGE_STORAGE, BUFFER_MEMORY_BOTH);
    
    return id;
}

//---------------------------------------------------
//: Link reserved buffer
//      Creates a pointer to the relevant bufferdata in the reserved buffers list
//---------------------------------------------------
void Buffer::linkReservedBuffer(str name, const BufferData* buffer) {
    for (auto &r : reserved_buffers) {
        if (r.name == name) {
            r.buffer = buffer;
            return;
        }
    }
    log::error("Invalid buffer name %s", name.c_str());
}

//---------------------------------------------------
//: Create a block buffer
//      All of the sizes are in items, they must all be multiplied by stride when talking about memory
//---------------------------------------------------
BlockBuffer Buffer::createBlockBuffer(ui32 initial_size, ui32 stride, BufferUsage usage, BufferMemory memory,
                                      std::function<void(BufferData &buffer)> callback) {
    BlockBuffer buffer{};
    
    //: Buffer parameters
    buffer.stride = stride;
    buffer.allocation_chunk = initial_size;
    buffer.usage = usage;
    
    //: Allocate buffer
    buffer.buffer = Buffer::API::allocateBuffer(initial_size * stride, usage, memory);
    
    //: Blocks
    buffer.blocks = {};
    buffer.free_blocks = {{initial_size, 0, {}}};
    
    //: Callback
    buffer.callback = callback;
    
    return buffer;
}

//---------------------------------------------------
//:
//---------------------------------------------------
ui32 Buffer::addToBlockBuffer(BlockBuffer &buffer, ui32 block, void* data, ui32 count, bool exact) {
    //: There is no block, create a new one
    if (not buffer.blocks.count(block)) {
        buffer.blocks[block] = BlockBufferPartition{0, 0, {0}};
        growBlockBuffer(buffer, block, count, exact);
    }
    
    //: Get new index inside the block
    auto &free_pos = buffer.blocks.at(block).free_positions;
    ui32 index = free_pos.at(0);
    
    //: Update the free positions for this block
    if (free_pos.size() == 1) {
        free_pos.at(0) += count;
        if (free_pos.at(0) > buffer.blocks.at(block).size)
            growBlockBuffer(buffer, block, count, exact);
    } else {
         free_pos.erase(free_pos.begin());
    }
    
    //: Add value to buffer
    if (data != nullptr)
        Buffer::API::updateBuffer(buffer.buffer, data, count * buffer.stride, (buffer.blocks.at(block).offset + index) * buffer.stride);
    
    return index;
}

//---------------------------------------------------
//:
//---------------------------------------------------
void Buffer::removeFromBlockBuffer(BlockBuffer &buffer, ui32 block, ui32 index) {
    //: Check if block exists
    if (not buffer.blocks.count(block))
        log::error("Removing non existent block from buffer");
    
    //: Add the index to the free positions so it can be overwritter
    buffer.blocks.at(block).free_positions.push_back(index);
    
    //: Sort the free positions
    std::sort(buffer.blocks.at(block).free_positions.begin(), buffer.blocks.at(block).free_positions.end());
}

//---------------------------------------------------
//:
//---------------------------------------------------
void flattenFreeBlocks(std::vector<BlockBufferPartition> &free_blocks) {
    //: Sort
    std::sort(free_blocks.begin(), free_blocks.end(), [](auto &a, auto &b){ return a.offset < b.offset; });
    
    //: New flat vector
    std::vector<BlockBufferPartition> flat_blocks = { free_blocks.at(0) };
    
    //: Flatten
    for (int i = 1; i < free_blocks.size(); i++) {
        if (free_blocks.at(i).offset == flat_blocks.rbegin()->offset + flat_blocks.rbegin()->size)
            flat_blocks.rbegin()->size += free_blocks.at(i).size;
        else
            flat_blocks.push_back(free_blocks.at(i));
    }
    
    free_blocks = flat_blocks;
}

void Buffer::growBlockBuffer(BlockBuffer &buffer, ui32 block, ui32 size, bool exact) {
    auto &b = buffer.blocks.at(block);
    
    //: Closest whole multiple of the allocation chunk that fits size
    ui32 grow_size = exact ? size : std::ceil(std::max(buffer.allocation_chunk, size) / buffer.allocation_chunk) * buffer.allocation_chunk;
    ui32 new_size = grow_size + b.size;
    
    //: Add previous offset to free blocks, and flatten free blocks list
    if (b.size > 0)
        buffer.free_blocks.push_back(BlockBufferPartition{b.size, b.offset, {}});
    flattenFreeBlocks(buffer.free_blocks);
    
    //: Find new position inside the buffer
    std::optional<ui32> new_offset;
    for (int i = (int)buffer.free_blocks.size() - 1; i >= 0; i--) {
        if (buffer.free_blocks.at(i).size >= new_size) {
            //: Found suitable offset
            new_offset = buffer.free_blocks.at(i).offset;
            //: Update free block list
            if (i == buffer.free_blocks.size() - 1 or buffer.free_blocks.at(i).size > new_size) {
                buffer.free_blocks.at(i).offset += new_size;
                buffer.free_blocks.at(i).size -= new_size;
            } else {
                buffer.free_blocks.erase(buffer.free_blocks.begin() + i);
            }
            break;
        }
    }
    
    //: If there is no space left, grow buffer
    if (not new_offset.has_value()) {
        //: Calculate new size
        auto last_block = std::max_element(buffer.free_blocks.begin(), buffer.free_blocks.end(),
                                           [](auto &a, auto &b)->bool{ return a.offset + a.size < b.offset + b.size; } );
        ui32 new_buffer_size = last_block->offset + last_block->size + new_size;
        
        //: Allocate new buffer
        BufferData new_buffer = Buffer::API::allocateBuffer(new_buffer_size * buffer.stride, buffer.usage, buffer.buffer.memory);
        
        //: Copy from previous buffer and delete previous one
        if (last_block->offset + last_block->size > 0) {
            Buffer::API::copyBuffer(buffer.buffer, new_buffer, (last_block->offset + last_block->size) * buffer.stride, 0);
            Buffer::API::destroyBuffer(buffer.buffer);
            buffer_list.erase(buffer.buffer);
        }
        
        //: Update free blocks
        buffer.buffer = new_buffer;
        new_offset = last_block->offset;
        last_block->offset += new_size;
        
        //: Callback actions when the buffer is expanded and changes reference
        buffer.callback(buffer.buffer);
    }
    
    //: Copy block if growing
    if (b.size > 0 and b.offset != new_offset.value())
        Buffer::API::copyBuffer(buffer.buffer, buffer.buffer, b.size * buffer.stride, new_offset.value() * buffer.stride, b.offset * buffer.stride);
    
    //: Update block info
    b.size = new_size;
    b.offset = new_offset.value();
}
