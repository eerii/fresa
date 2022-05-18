//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_draw.h"
#include "f_math.h"
#include "r_api.h"
#include "r_vulkan_api.h" //: TODO: REMOVE THIS
#include <sstream>

using namespace Fresa;
using namespace Graphics;

constexpr ui32 draw_batch_chunk = 1024 * sizeof(DrawInstanceData);
constexpr ui32 draw_batch_buffer_grow_amount = 4 * draw_batch_chunk;
constexpr ui32 no_draw_block_offset = UINT_MAX;

//---------------------------------------------------
//: Register draw command
//---------------------------------------------------
DrawCommandID Draw::registerDrawCommand(DrawQueueObject draw) {
    //: Check if block buffer is available
    if (draw_commands.buffer.buffer == VK_NULL_HANDLE)
        draw_commands = Buffer::createBlockBuffer(256, draw_command_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    //: Create indirect draw command
    std::vector<VkDrawIndexedIndirectCommand> cmd(1);
    cmd.at(0).indexCount = meshes.index.blocks.at(draw.mesh.index).size;
    cmd.at(0).instanceCount = draw.instance_count;
    cmd.at(0).firstIndex = meshes.index.blocks.at(draw.mesh.index).offset;
    cmd.at(0).vertexOffset = meshes.vertex.blocks.at(draw.mesh.vertex).offset;
    cmd.at(0).firstInstance = draw.instance_offset;
    
    //: Add to block buffer
    return Buffer::addToBlockBuffer(draw_commands, 0, (void*)cmd.data(), (ui32)cmd.size());
}

//---------------------------------------------------
//: Register mesh
//      Adds the mesh data to the mesh buffers with the appropiate offset and padding
//      Also checks if the buffer is too small and extends it otherwise
//      Returns a MeshID for referencing during draw
//---------------------------------------------------
MeshID Draw::registerMeshInternal(void* vertices, ui32 vertices_size, ui8 vertex_bytes, void* indices, ui32 indices_size, ui8 index_bytes) {
    MeshID id{};
    
    //: Check if block buffers are created, if not, allocate them
    if (meshes.vertex.buffer.buffer == VK_NULL_HANDLE)
        meshes.vertex = Buffer::createBlockBuffer(4092, vertex_bytes, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_VERTEX), BUFFER_MEMORY_GPU_ONLY);
    if (meshes.index.buffer.buffer == VK_NULL_HANDLE)
        meshes.index = Buffer::createBlockBuffer(4092, index_bytes, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDEX), BUFFER_MEMORY_GPU_ONLY);
    
    //: Check index bytes
    if (not (index_bytes == 2 or index_bytes == 4))
        log::error("Index buffers support ui16 (2 bytes) and ui32 (4 bytes) formats, but you loaded one index with %d bytes", index_bytes);
    
    //: Add data to buffers
    id.vertex = (ui32)meshes.vertex.blocks.size();
    if (Buffer::addToBlockBuffer(meshes.vertex, id.vertex, vertices, vertices_size, true) != 0)
        log::error("You are allocating two meshes with the same id");
    id.index = (ui32)meshes.index.blocks.size();
    if (Buffer::addToBlockBuffer(meshes.index, id.index, indices, indices_size, true) != 0)
        log::error("You are allocating two meshes with the same id");
    
    log::info("%d %d", meshes.vertex.blocks.at(id.vertex).size, meshes.vertex.blocks.at(id.vertex).offset);
    
    return id;
}

//---------------------------------------------------
//: Register draw id
//      Creates a new draw description using some parameters
//      Also creates a draw batch by hashing those parameters to group compatible draws
//      Adds it to the batches buffer and calculates the batch offset
//---------------------------------------------------
DrawID Draw::registerDrawID(MeshID mesh) {
    //: Find new id
    DrawID id = draw_scene.objects.size() == 0 ? DrawID{} : draw_scene.objects.end()->first;
    do { id.value++; } while (draw_scene.objects.count(id));
    
    //: Create the draw object
    DrawDescription draw{};
    draw.mesh = mesh;
    
    //: Calculate draw batch hash
    //      For now it uses the mesh id, but in the future it can reference more properties
    std::stringstream ss;
    ss << draw.mesh.vertex;
    ss << draw.mesh.index;
    auto s = ss.str();
    draw.batch = DrawBatchID{Math::hash(s.c_str(), s.size())};
    
    //: Register batch if not created yet
    if (not draw_scene.batches.count(draw.batch))
        allocateSceneBatchBlock(draw.batch);
    
    //: Grow the batch block if there is no space left
    auto &batch = draw_scene.batches.at(draw.batch);
    if (batch.free_positions.size() == 1 and (batch.free_positions.at(0) + 1) * sizeof(DrawInstanceData) >= batch.size)
        allocateSceneBatchBlock(draw.batch);
    
    //: Get the batch offset for this new object and update the free positions
    draw.batch_offset = batch.free_positions.at(0);
    if (batch.free_positions.size() == 1)
        batch.free_positions.at(0) += 1;
    else
        batch.free_positions.erase(batch.free_positions.begin());
    
    //: Add to scene objects and to the recreation queue
    draw_scene.objects[id] = draw;
    
    return id;
}


//---------------------------------------------------
//: Draw
//      Adds a DrawID to the specified shader's render queue for draw indirect command creation
//      Count adds that number of contiguous draw calls to the instance count
//---------------------------------------------------
void Draw::draw(ShaderID shader, DrawID id, ui32 count) {
    //: Check if the DrawID is valid
    if (not draw_scene.objects.count(id))
        log::error("Using invalid DrawID (%d)", id.value);
    auto &object = draw_scene.objects.at(id);
    
    //: Add to draw queue (first item)
    auto it = std::find_if(draw_queue.begin(), draw_queue.end(), [&](const auto &d){
        return d.shader == shader and d.batch == object.batch;
    });
    if (it == draw_queue.end()) {
        DrawQueueObject new_draw{};
        new_draw.shader = shader;
        new_draw.batch = object.batch;
        new_draw.mesh = object.mesh;
        new_draw.instance_count = count;
        new_draw.instance_offset = draw_scene.batches.at(object.batch).offset;
        draw_queue.push_back(new_draw);
    }
    //: Add to draw queue (next items)
    else {
        it->instance_count += count;
    }
}

//---------------------------------------------------
//: Allocate batch block
//      Creates or expands batch block. Keeps the contents on expand and tightly packs the buffer
//      TODO: Make it so you can specify the draw batch chunk when getting the drawIDs
//      TODO: individual vs instanced too, mesh type, group by update frequency and number of instances
//---------------------------------------------------
void Draw::allocateSceneBatchBlock(DrawBatchID batch) {
    DrawBatchBlock new_block{};
    DrawBatchBlock previous_block{};
    new_block.offset = no_draw_block_offset;
    
    //: There is no batch block, create a new one
    if (not draw_scene.batches.count(batch)) {
        new_block.size = draw_batch_chunk;
        new_block.free_positions = { 0 };
    }
    //: There is a batch block, grow it
    else {
        previous_block = draw_scene.batches.at(batch);
        new_block.size = previous_block.size + draw_batch_chunk;
        new_block.free_positions = previous_block.free_positions;
    }
    
    //: Add previous offset to free blocks, and flatten free blocks list
    if (previous_block.size > 0)
        draw_scene.free_blocks.push_back(DrawBatchBlock{previous_block.offset, previous_block.size, {0}});
    std::sort(draw_scene.free_blocks.begin(), draw_scene.free_blocks.end(), [](auto &a, auto &b){ return a.offset < b.offset; });
    std::vector<DrawBatchBlock> flat_free_blocks = { *draw_scene.free_blocks.begin() };
    for (int i = 1; i < draw_scene.free_blocks.size(); i++) {
        if (draw_scene.free_blocks.at(i).offset == flat_free_blocks.rbegin()->offset + flat_free_blocks.rbegin()->size)
            flat_free_blocks.rbegin()->size += draw_scene.free_blocks.at(i).size;
        else
            flat_free_blocks.push_back(draw_scene.free_blocks.at(i));
    }
    draw_scene.free_blocks = flat_free_blocks;
    
    //: Find new offset
    while (new_block.offset == no_draw_block_offset) {
        for (int i = 0; i < draw_scene.free_blocks.size(); i++) {
            if (draw_scene.free_blocks.at(i).size >= new_block.size) {
                new_block.offset = draw_scene.free_blocks.at(i).offset;
                //: Update free block list
                if (i == draw_scene.free_blocks.size() - 1 or draw_scene.free_blocks.at(i).size > new_block.size) {
                    draw_scene.free_blocks.at(i).offset += new_block.size;
                    draw_scene.free_blocks.at(i).size -= new_block.size;
                } else {
                    draw_scene.free_blocks.erase(draw_scene.free_blocks.begin() + i);
                }
                break;
            }
        }
        
        //: Grow buffer if there is not enough space left
        if (new_block.offset == no_draw_block_offset) {
            //: Calculate new size
            auto last_block = std::max_element(draw_scene.free_blocks.begin(), draw_scene.free_blocks.end(),
                                               [](auto &a, auto &b)->bool{ return a.offset + a.size < b.offset + b.size; } );
            ui32 new_buffer_size = last_block->offset + last_block->size + draw_batch_buffer_grow_amount;
            while (new_buffer_size - last_block->offset < new_block.size)
                new_buffer_size += draw_batch_buffer_grow_amount;
            
            //: Allocate new buffer
            BufferData new_buffer = Common::allocateBuffer(new_buffer_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_BOTH);
            
            //: TODO: Move this to a GPU buffer and create a double buffer with staging
            
            //: Copy from previous buffer and delete previous one
            if (last_block->offset + last_block->size > 0) {
                Common::copyBuffer(draw_scene.buffer, new_buffer, last_block->offset + last_block->size, 0);
                Common::destroyBuffer(draw_scene.buffer);
                buffer_list.erase(draw_scene.buffer);
            }
            
            //: Update scene buffer and free blocks
            draw_scene.buffer = new_buffer;
            draw_scene.free_blocks.rbegin()->size += draw_batch_buffer_grow_amount;
            
            //: Link new buffer to shaders that use it
            for (auto &[id, shader] : shaders.list.at(SHADER_DRAW)) {
                for (auto &d : shader.descriptors) {
                    for (auto &res : d.resources) {
                        if (res.type != DESCRIPTOR_STORAGE)
                            continue;
                        auto r_id = std::get<StorageBufferID>(res.id);
                        if (reserved_buffers.count(r_id) and reserved_buffers.at(r_id) == "InstanceBuffer")
                            Common::linkDescriptorResources(id);
                    }
                }
            }
            
            //: Map the buffer to the cpu
            vmaMapMemory(api.allocator, draw_scene.buffer.allocation, &draw_scene.buffer_data);
        }
    }
    
    //: Copy block if growing
    if (new_block.size > draw_batch_chunk)
        Common::copyBuffer(draw_scene.buffer, draw_scene.buffer, previous_block.size, new_block.offset, previous_block.offset);
    
    //: Save to draw scene
    draw_scene.batches[batch] = new_block;
}

//---------------------------------------------------
//: Get instance data
//      Returns a pointer to the draw scene buffer for the specified draw id data
//      If count is specified, it will also check that there is sufficient draw ids specified for the query to be valid (using the pointer as an array)
//      However, there is no guarantee that all intermediate objects are valid DrawIDs. In the future, it would be nice to make sure that
//      when adding/removing draw descriptions, they are always tightly packed so this query can be always successful
//---------------------------------------------------
DrawInstanceData* Draw::getInstanceData(DrawID id, ui32 count) {
    //: Check that the DrawID is valid
    if (not draw_scene.objects.count(id))
        log::error("Invalid Draw ID %d", id.value);
    if (count > 1 and not draw_scene.objects.count(id + (count - 1)))
        log::error("Invalid Draw ID range (%d-%d)", id.value, id.value + count - 1);
    
    //: Check if the buffer is mapped
    if (draw_scene.buffer_data == nullptr)
        log::error("The draw scene buffer is not mapped into memory");
    
    //: Return a pointer to the correct offset to the mapped buffer data
    auto &object = draw_scene.objects.at(id);
    auto &batch = draw_scene.batches.at(object.batch);
    return (DrawInstanceData*)draw_scene.buffer_data + object.batch_offset + batch.offset / sizeof(DrawInstanceData);
}

//---------------------------------------------------
//: Build draw queue
//---------------------------------------------------
void Draw::buildDrawQueue() {
    //: New draws
    ui32 new_draw_count = 0;
    for (auto &d : draw_queue) {
        auto it = std::find_if(previous_draw_queue.begin(), previous_draw_queue.end(), [&d](const auto &i){
            return i.shader == d.shader and i.batch == d.batch and i.instance_count == d.instance_count;
        });
        if (it == previous_draw_queue.end()) {
            d.command = registerDrawCommand(d);
            built_draw_queue[d.shader].push_back(d.command);
            new_draw_count++;
        }
    }
    
    //: Removed draws
    /*if (previous_draw_queue.size() + new_draw_count > draw_queue.size()) {
        for (auto &d : previous_draw_queue) {
            auto it = std::find_if(draw_queue.begin(), draw_queue.end(), [&d](const auto &i){
                return i.shader == d.shader and i.batch == d.batch and i.instance_count == d.instance_count;
            });
            if (it == draw_queue.end()) {
                removeDrawCommand(d.command);
                auto it_ = std::find_if(built_draw_queue.at(d.shader).begin(), built_draw_queue.at(d.shader).end(), [&d](const auto &i){
                    return i.value == d.command.value;
                });
                built_draw_queue.at(d.shader).erase(it_);
                if (built_draw_queue.at(d.shader).size() == 0)
                    built_draw_queue.erase(d.shader);
            }
        }
    }*/
    
    previous_draw_queue = draw_queue;
    draw_queue.clear();
}
