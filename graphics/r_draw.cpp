//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_draw.h"
#include "f_math.h"
#include <sstream>

using namespace Fresa;
using namespace Graphics;

constexpr ui32 draw_command_pool_chunk = 256 * draw_command_size;
constexpr ui32 draw_batch_chunk = 1024 * sizeof(glm::mat4);
constexpr ui32 draw_batch_buffer_grow_amount = 4 * draw_batch_chunk;

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
DrawID Draw::registerDrawID(MeshID mesh, glm::mat4 initial_transform) {
    //: Find new id
    DrawID id = draw_scene.objects.size() == 0 ? DrawID{} : draw_scene.objects.end()->first;
    do { id.value++; } while (draw_scene.objects.count(id));
    
    //: Create the draw object
    DrawDescription draw{};
    draw.mesh = mesh;
    draw.transform = initial_transform;
    
    //: Calculate draw batch hash
    //      For now it uses the mesh id, but in the future it can reference more properties
    std::stringstream ss;
    ss << draw.mesh.value;
    auto s = ss.str();
    draw.batch = DrawBatchID{Math::hash(s.c_str(), s.size())};
    
    //: Add to scene objects and to the recreation queue
    draw_scene.objects[id] = draw;
    draw_scene.recreate_objects[id] = draw;
    
    return id;
}


//---------------------------------------------------
//:
//---------------------------------------------------
void Draw::draw(DrawID id, ShaderID shader, glm::mat4 transform) {
    if (not draw_scene.objects.count(id))
        log::error("Using invalid DrawID (%d)", id.value);
    temp_draw_queue[shader] = TempDrawObject{draw_scene.objects.at(id).mesh, DrawCommandID{0}};
}

//---------------------------------------------------
//:
//---------------------------------------------------
void Draw::allocateSceneBatchBlock(DrawBatchID batch) {
    DrawBatchBlock new_block{};
    DrawBatchBlock previous_block{};
    new_block.offset = no_draw_batch_offset;
    
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
    while (new_block.offset == no_draw_batch_offset) {
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
        if (new_block.offset == no_draw_batch_offset) {
            //: Calculate new size
            auto last_block = std::max_element(draw_scene.free_blocks.begin(), draw_scene.free_blocks.end(),
                                               [](auto &a, auto &b)->bool{ return a.offset + a.size < b.offset + b.size; } );
            ui32 new_buffer_size = last_block->offset + last_block->size + draw_batch_buffer_grow_amount;
            while (new_buffer_size - last_block->offset < new_block.size)
                new_buffer_size += draw_batch_buffer_grow_amount;
            
            //: Allocate new buffer
            BufferData new_buffer = Common::allocateBuffer(new_buffer_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_GPU_ONLY);
            
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
void Draw::compileSceneBatches() {
    std::map<ui32, glm::mat4> copy_list{};
    
    //: Iterate through all the objects that need reuploading
    for (auto &[obj_id, object] : draw_scene.recreate_objects) {
        //: If there is no matching batch id in the scene, create a new one
        if (not draw_scene.batches.count(object.batch)) {
            allocateSceneBatchBlock(object.batch);
        }
        auto &batch = draw_scene.batches.at(object.batch);
        
        //: If the object has no batch offset, find one
        if (object.batch_offset == no_draw_batch_offset) {
            //: Grow the batch block if there is no space left
            if (batch.free_positions.size() == 1 and batch.free_positions.at(0) + sizeof(glm::mat4) >= batch.size)
                allocateSceneBatchBlock(object.batch);
            
            //: Get the batch offset for this new object and update the free positions
            object.batch_offset = draw_scene.batches.at(object.batch).free_positions.at(0);
            if (batch.free_positions.size() == 1)
                batch.free_positions.at(0) += sizeof(glm::mat4);
            else
                batch.free_positions.erase(batch.free_positions.begin());
        }
        
        //: Add to copy list
        copy_list[batch.offset + object.batch_offset] = object.transform;
    }
    
    if (copy_list.size() > 0) {
        //: Flatten copy list
        std::map<ui32, std::vector<glm::mat4>> flat_copy_list = { {copy_list.begin()->first, {copy_list.begin()->second}} };
        for (auto &[offset, transform] : copy_list) {
            if (offset == copy_list.begin()->first)
                continue;
            if (offset == flat_copy_list.rbegin()->first + flat_copy_list.rbegin()->second.size() * sizeof(glm::mat4))
                flat_copy_list.rbegin()->second.push_back(transform);
            else
                flat_copy_list[offset] = {transform};
        }
        
        //: Update buffer
        for (auto &[offset, transforms] : flat_copy_list)
            Common::updateBuffer(draw_scene.buffer, (void*)transforms.data(), (ui32)(transforms.size() * sizeof(glm::mat4)), offset);
    }
    
    draw_scene.recreate_objects.clear();
}
