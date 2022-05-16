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

constexpr ui32 draw_command_pool_chunk = 256 * draw_command_size;
constexpr ui32 draw_batch_chunk = 1024 * sizeof(DrawInstanceData); //TODO: Make it so you can specify the draw batch chunk when getting the drawIDs, individual vs instanced too, mesh type, group by update frequency and number of instances
constexpr ui32 draw_batch_buffer_grow_amount = 4 * draw_batch_chunk;
constexpr ui32 no_draw_block_offset = UINT_MAX;

//---------------------------------------------------
//: Allocate draw indirect command buffer
//      Create a new draw indirect buffer expanding the previous size and, if existent, copy the previous contents
//---------------------------------------------------
DrawCommandBuffer Draw::allocateDrawCommandBuffer() {
    DrawCommandBuffer new_draw_commands;
    
    //: Expand the buffer size by one chunk
    ui32 previous_size = draw_commands.pool_size;
    new_draw_commands.pool_size = previous_size + draw_command_pool_chunk;
    
    //: Allocate the draw command buffers
    new_draw_commands.buffer = Common::allocateBuffer(new_draw_commands.pool_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    //: Sort the free positions
    std::sort(draw_commands.free_positions.begin(), draw_commands.free_positions.end(), [](auto &a, auto &b)->bool{ return a.value < b.value; });
    
    //: If it has contents, copy them to the new buffers
    //      Finds the last element (the one with the highest offset), then copies the buffer from the beginning to the offset + size of the last element
    //      Should be reasonably efficient providing the elements are tightly packed
    if (draw_commands.free_positions.rbegin()->value > 0) {
        //: Copy the previous command buffer
        Common::copyBuffer(draw_commands.buffer, new_draw_commands.buffer, draw_commands.free_positions.end()->value * draw_command_size);
        
        //: Copy free positions
        new_draw_commands.free_positions = draw_commands.free_positions;
    }
    
    return new_draw_commands;
}

//---------------------------------------------------
//: Register draw command
//---------------------------------------------------
DrawCommandID Draw::registerDrawCommand(MeshID mesh) {
    //: Get id from free positions
    DrawCommandID id{draw_commands.free_positions.begin()->value};
    
    //: Check if the id is out of bounds and recreate the command buffer
    if ((id.value + 1) * draw_command_size > draw_commands.pool_size) {
        draw_commands = allocateDrawCommandBuffer();
        if (draw_commands.pool_size > draw_command_pool_chunk)
            log::warn("Growing draw command buffer more than one pool chunk, consider increasing it for efficiency");
    }
    
    //: Update free positions
    if (id.value == draw_commands.free_positions.rbegin()->value)
        draw_commands.free_positions.rbegin()->value++;
    else
        draw_commands.free_positions.erase(draw_commands.free_positions.begin());
    
    //: Create indirect draw command
    std::vector<VkDrawIndexedIndirectCommand> cmd(1);
    cmd.at(0).indexCount = meshes.paddings.at(mesh).index_size;
    cmd.at(0).instanceCount = 1000;
    cmd.at(0).firstIndex = meshes.paddings.at(mesh).index_last - meshes.paddings.at(mesh).index_size;
    cmd.at(0).vertexOffset = meshes.paddings.at(mesh).vertex_last - meshes.paddings.at(mesh).vertex_size;
    cmd.at(0).firstInstance = 0; //instance.instance_count * i++;
    
    //TODO: HANDLE MULTIPLE VERTEX FORMATS
    
    //: Add to buffer
    Common::updateBuffer(draw_commands.buffer, (void*)cmd.data(), draw_command_size, id.value * draw_command_size);
    
    return id;
}

//---------------------------------------------------
//: Remove draw command
//---------------------------------------------------
void Draw::removeDrawCommand(DrawCommandID id) {
    //: Add id to free positions
    draw_commands.free_positions.push_back(id);
    
    //: Sort the draw commands
    std::sort(draw_commands.free_positions.begin(), draw_commands.free_positions.end(), [](auto &a, auto &b)->bool{ return a.value < b.value; });
}

//---------------------------------------------------
//:
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
    ss << draw.mesh.value;
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
//:
//---------------------------------------------------
void Draw::draw(ShaderID shader, DrawID id) {
    //: Check if the DrawID is valid
    if (not draw_scene.objects.count(id))
        log::error("Using invalid DrawID (%d)", id.value);
    
    //: TEMP TODO: NEXT, KEEP HERE THE INSTANCE COUNT, OFFSETS, ... WITH THE DRAW COMMAND
    temp_draw_queue[shader] = TempDrawObject{draw_scene.objects.at(id).mesh, DrawCommandID{0}};
}

//---------------------------------------------------
//:
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
            
            //: Copy from previous buffer
            if (last_block->offset + last_block->size > 0)
                Common::copyBuffer(draw_scene.buffer, new_buffer, last_block->offset + last_block->size, 0);
            
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
            VK::deletion_queue_program.push_back([](){ vmaUnmapMemory(api.allocator, draw_scene.buffer.allocation); });
            
            //: TODO: Delete previous buffer
        }
    }
    
    //: Copy block if growing
    if (new_block.size > draw_batch_chunk)
        Common::copyBuffer(draw_scene.buffer, draw_scene.buffer, previous_block.size, new_block.offset, previous_block.offset);
    
    //: Save to draw scene
    draw_scene.batches[batch] = new_block;
}

//---------------------------------------------------
//:
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
