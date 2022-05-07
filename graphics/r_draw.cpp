//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_draw.h"

using namespace Fresa;
using namespace Graphics;

constexpr ui32 draw_command_pool_chunk = 256 * draw_command_size;
constexpr ui32 draw_object_chunk = 64000;

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
DrawObjectBuffer Draw::allocateDrawObjectBuffer(std::vector<DrawBatchID> batches, ui32 object_size) {
    DrawObjectBuffer new_draw_objects;
    ui32 buffer_size = 0;
    std::vector<std::pair<DrawObjectBlock, DrawObjectBlock>> copy_list{};
    
    //: Copy the free blocks
    new_draw_objects.free_blocks = draw_objects.free_blocks;
    
    //: Save the stride of each object (for example, sizeof(glm::mat4))
    if (object_size == 0) {
        if (draw_objects.stride == 0)
            log::error("You need to provide an object stride to the allocateDrawObjectBuffer at least the first time");
        new_draw_objects.stride = draw_objects.stride;
    } else {
        new_draw_objects.stride = object_size;
    }
    
    //: Iterate through all batches
    for (auto &b : batches) {
        new_draw_objects.batch_offsets[b] = DrawObjectBlock{};
        
        //: The batch already exists, add one chunk
        if (draw_objects.batch_offsets.count(b)) {
            //: Size
            new_draw_objects.batch_offsets.at(b).size = draw_objects.batch_offsets.at(b).size + draw_object_chunk;
            //: Offset
            for (auto &fb : new_draw_objects.free_blocks)
                if (fb.size >= new_draw_objects.batch_offsets.at(b).size)
                    new_draw_objects.batch_offsets.at(b).offset = fb.offset;
            if (new_draw_objects.batch_offsets.at(b).offset == 0)
                new_draw_objects.batch_offsets.at(b).offset = new_draw_objects.free_blocks.rbegin()->offset;
            //: Update free blocks
            if (new_draw_objects.batch_offsets.at(b).offset == new_draw_objects.free_blocks.rbegin()->offset) {
                new_draw_objects.free_blocks.rbegin()->offset += new_draw_objects.batch_offsets.at(b).size;
                new_draw_objects.free_blocks.rbegin()->size = draw_object_chunk;
            } else {
                for (int i = 0; i < new_draw_objects.free_blocks.size(); i++) {
                    if (new_draw_objects.free_blocks.at(i).offset != new_draw_objects.batch_offsets.at(b).offset)
                        continue;
                    if (new_draw_objects.free_blocks.at(i).size == new_draw_objects.batch_offsets.at(b).size) {
                        new_draw_objects.free_blocks.erase(new_draw_objects.free_blocks.begin() + i);
                        break;
                    }
                    new_draw_objects.free_blocks.at(i).size -= new_draw_objects.batch_offsets.at(b).size;
                    new_draw_objects.free_blocks.at(i).offset += new_draw_objects.batch_offsets.at(b).size;
                }
            }
            new_draw_objects.free_blocks.push_back(DrawObjectBlock{draw_objects.batch_offsets.at(b).offset, draw_objects.batch_offsets.at(b).size, {0}});
            std::sort(new_draw_objects.free_blocks.begin(), new_draw_objects.free_blocks.end(), [](auto &a, auto &b){ return a.offset < b.offset; });
            //: Copy list
            copy_list.push_back({DrawObjectBlock{draw_objects.batch_offsets.at(b).offset, draw_objects.batch_offsets.at(b).size, {0}},
                                 DrawObjectBlock{new_draw_objects.batch_offsets.at(b).offset, draw_objects.batch_offsets.at(b).size, {0}}});
        }
        
        //: New batch
        else {
            //: Size
            new_draw_objects.batch_offsets.at(b).size = draw_object_chunk;
            //: Offset
            new_draw_objects.batch_offsets.at(b).offset = new_draw_objects.free_blocks.begin()->offset;
            //: Update free blocks
            if (new_draw_objects.free_blocks.size() == 1) {
                new_draw_objects.free_blocks.begin()->offset += draw_object_chunk;
                new_draw_objects.free_blocks.begin()->size = draw_object_chunk;
            } else {
                new_draw_objects.free_blocks.erase(new_draw_objects.free_blocks.begin());
            }
        }
        
        //: Calculate the total size of the buffer
        buffer_size += new_draw_objects.batch_offsets.at(b).size;
    }
    
    //: Iterate through the original buffers that are not modified
    for (auto &[b_id, b] : draw_objects.batch_offsets) {
        if (not std::count(batches.begin(), batches.end(), b_id)) {
            //: Batch offsets
            new_draw_objects.batch_offsets[b_id] = b;
            //: Copy list
            copy_list.push_back({DrawObjectBlock{b.offset, b.size, {0}},
                                 DrawObjectBlock{b.offset, b.size, {0}}});
            
            buffer_size += b.size;
        }
    }
    
    //: Allocate new buffer
    new_draw_objects.buffer = Common::allocateBuffer(buffer_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_GPU_ONLY);
    
    if (copy_list.size() > 0) {
        //: Flatten copy list
        std::vector<std::pair<DrawObjectBlock, DrawObjectBlock>> flat_copy_list = { *copy_list.begin() };
        for (int i = 1; i < copy_list.size(); i++) {
            if (copy_list.at(i).first.offset == flat_copy_list.rbegin()->first.offset + flat_copy_list.rbegin()->first.size and
                copy_list.at(i).second.offset == flat_copy_list.rbegin()->second.offset + flat_copy_list.rbegin()->second.size) {
                flat_copy_list.rbegin()->first.size += copy_list.at(i).first.size;
                flat_copy_list.rbegin()->second.size += copy_list.at(i).second.size;
            } else {
                flat_copy_list.push_back(copy_list.at(i));
            }
        }
        
        //: Copy
        for (auto &cl : flat_copy_list)
            Common::copyBuffer(draw_objects.buffer, new_draw_objects.buffer, cl.second.size, cl.second.offset, cl.first.offset);
    }
    
    return new_draw_objects;
}

//---------------------------------------------------
//:
//---------------------------------------------------
DrawID Draw::registerDrawID(ShaderID shader, MeshID mesh, glm::mat4 initial_transform) {
    //: Create draw scene if not present
    if (not draw_scenes.count(shader))
        draw_scenes[shader] = DrawScene{};
    
    //: Draw scene objects
    auto& scene_objects = draw_scenes.at(shader).objects;
    
    //: Find new id
    DrawID id = scene_objects.size() == 0 ? DrawID{} : scene_objects.end()->first;
    do { id.value++; } while (scene_objects.count(id));
    
    //: Create the draw object
    DrawDescription draw{};
    draw.shader = shader;
    draw.mesh = mesh;
    draw.transform = initial_transform;
    
    //: Add to scene objects and to the recreation queue
    scene_objects[id] = draw;
    draw_scenes.at(shader).recreate_objects[id] = draw;
    
    return id;
}


//---------------------------------------------------
//:
//---------------------------------------------------
void Draw::draw(DrawID id, glm::mat4 transform) {
    for (auto &[shader, scene] : draw_scenes) {
        if (scene.objects.count(id))
            temp_draw_queue[shader] = TempDrawObject{scene.objects.at(id).mesh, DrawCommandID{0}};
    }
}

//---------------------------------------------------
//:
//---------------------------------------------------
void Draw::compileSceneBatches(DrawScene &scene) {
    //: Iterate through all the objects that need reuploading
    for (auto &[obj_id, object] : scene.recreate_objects) {
        //: Find the appropiate draw batch for the object
        //      Right now it only checks if the mesh is the same, but in the future more checks might be done, for example, a material check
        std::optional<DrawBatchID> batch_id;
        for (auto &[id, batch] : scene.batches) {
            if (batch.mesh != object.mesh)
                continue;
            batch_id = id;
            break;
        }
        
        //: If no batch is found, create a new one
        if (not batch_id.has_value()) {
            //: Find id
            DrawBatchID id = scene.batches.size() == 0 ? DrawBatchID{} : scene.batches.end()->first;
            do { id.value++; } while (scene.batches.count(id));
            batch_id = id;
            
            //: Get 
            
            //: Add to list
            scene.batches[id] = DrawBatch{object.mesh, 0, 1};
        }
    }
    
    scene.recreate_objects.clear();
}
