//: fresa by jose pazos perez, licensed under GPLv3
#include "r_draw.h"
#include "r_api.h"
#include "f_math.h"
#include <sstream>

using namespace Fresa;
using namespace Graphics;

//---------------------------------------------------
//: Register draw command
//---------------------------------------------------
void Draw::registerDrawCommand(ui32 hash, MeshID mesh, ui32 instance_count, ui32 instance_offset) {
    //: Check if block buffer is available
    if (draw_commands.buffer.buffer == no_buffer)
        draw_commands = Buffer::createBlockBuffer(256, sizeof(IDrawIndexedIndirectCommand), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    IDrawIndexedIndirectCommand cmd;
    cmd.indexCount = meshes.index.blocks.at(mesh).size;
    cmd.instanceCount = instance_count;
    cmd.firstIndex = meshes.index.blocks.at(mesh).offset;
    cmd.vertexOffset = meshes.vertex.blocks.at(mesh).offset;
    cmd.firstInstance = instance_offset;
    
    //: Update buffer
    if (draw_commands.blocks.count(hash))
        Buffer::API::updateBuffer(draw_commands.buffer, (void*)&cmd, sizeof(IDrawIndexedIndirectCommand), draw_commands.blocks.at(hash).offset * sizeof(IDrawIndexedIndirectCommand));
    //: Add new command to buffer
    else
        Buffer::addToBlockBuffer(draw_commands, hash, (void*)&cmd, 1, true);
}

//---------------------------------------------------
//: Register mesh
//      Adds the mesh data to the mesh buffers with the appropiate offset and padding
//      Also checks if the buffer is too small and extends it otherwise
//      Returns a MeshID for referencing during draw
//---------------------------------------------------
MeshID Draw::registerMeshInternal(void* vertices, ui32 vertices_size, ui8 vertex_bytes, void* indices, ui32 indices_size, ui8 index_bytes) {
    MeshID id;
    
    //: Check if block buffers are created, if not, allocate them
    if (meshes.vertex.buffer.buffer == no_buffer)
        meshes.vertex = Buffer::createBlockBuffer(1024, vertex_bytes, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_VERTEX), BUFFER_MEMORY_GPU_ONLY);
    if (meshes.index.buffer.buffer == no_buffer)
        meshes.index = Buffer::createBlockBuffer(1024, index_bytes, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDEX), BUFFER_MEMORY_GPU_ONLY);
    
    //: Check index bytes
    if (not (index_bytes == 2 or index_bytes == 4))
        log::error("Index buffers support ui16 (2 bytes) and ui32 (4 bytes) formats, but you loaded one index with %d bytes", index_bytes);
    
    //: Add data to buffers
    if (meshes.vertex.blocks.size() != meshes.index.blocks.size())
        log::error("There is a disparity between the vertex buffer (%d) and the index buffer (%d)", meshes.vertex.blocks.size(), meshes.index.blocks.size());
    id = (ui32)meshes.vertex.blocks.size();
    if (Buffer::addToBlockBuffer(meshes.vertex, id, vertices, vertices_size, true) != 0)
        log::error("You are allocating two meshes with the same id");
    if (Buffer::addToBlockBuffer(meshes.index, id, indices, indices_size, true) != 0)
        log::error("You are allocating two meshes with the same id");
    
    return id;
}

//---------------------------------------------------
//: Register material
//      Adds the material data to the material buffer, returns a MaterialID for referencing during draw
//---------------------------------------------------
MaterialID Draw::registerMaterial(glm::vec4 color) {
    //: Check if block buffer is available
    if (draw_materials.buffer.buffer == no_buffer)
        draw_materials = Buffer::createBlockBuffer(256, sizeof(DrawMaterialData), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_GPU_ONLY);
    
    //: Create material
    DrawMaterialData material{};
    material.color = color;
    
    //: Add to block buffer
    return Buffer::addToBlockBuffer(draw_materials, 0, (void*)&material, 1);
}

//---------------------------------------------------
//: Register draw id
//      Creates a new draw description using some parameters
//      Also creates a draw batch by hashing those parameters to group compatible draws
//      Adds it to the batches buffer and calculates the batch offset
//---------------------------------------------------
void linkReservedBuffers() {
    //: Link the new reserved buffer handles to the shaders that use them
    for (const auto &list : shaders.list) {
        for (const auto &[id, shader] : list) {
            [id=id, shader=shader](){
            for (const auto &d : shader.descriptors) {
                for (const auto &res : d.resources) {
                    auto it = std::find_if(reserved_buffers.begin(), reserved_buffers.end(), [&res](auto &b){ return b.id == res.id; });
                    if (it != reserved_buffers.end()) {
                        Shader::API::linkDescriptorResources(id);
                        return;
                    }
                }
            }}();
        }
    }
};

DrawID Draw::registerDrawID(MeshID mesh, MaterialID material, std::vector<ShaderID> draw_shaders, DrawBatchType type, ui32 count) {
    //: Make sure the transform block buffer is created
    if (draw_transform.buffer.buffer == no_buffer) {
        draw_transform = Buffer::createBlockBuffer(64, sizeof(DrawTransformData), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_BOTH, [](BufferData &buffer){
            //: Grow GPU buffer
            draw_transform_gpu = Buffer::API::allocateBuffer((draw_transform.free_blocks.rbegin()->offset + draw_transform.free_blocks.rbegin()->size) * sizeof(DrawTransformData), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_GPU_ONLY);
            
            //: Link new buffer handle to shaders that use it
            linkReservedBuffers();
            
            //: Map the buffer to the cpu
            vmaMapMemory(api.allocator, buffer.allocation, &draw_transform_data);
        });
    }
        
    //: Make sure the instance block buffer is created
    if (draw_instances.buffer.buffer == no_buffer) {
        draw_instances = Buffer::createBlockBuffer(64, sizeof(DrawInstanceData), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_GPU_ONLY, [](BufferData &buffer){ linkReservedBuffers(); });
    }
    
    //: Calculate the draw batch hash
    //      It takes into account the mesh (and possible more things in the future), separates static and dynamic objects and takes into account shaders
    std::stringstream ss;
    if (type & DRAW_STATIC)
        ss << DRAW_STATIC;
    if (type & DRAW_DYNAMIC)
        ss << DRAW_DYNAMIC;
    ss << mesh;
    for (auto &sh : draw_shaders)
        ss << sh;
    auto s = ss.str();
    ui32 batch = Math::hash(s.c_str(), s.size());
    
    //: Add transform
    ui32 offset = Buffer::addToBlockBuffer(draw_transform, batch, nullptr, count, false);
    
    //: Add instance
    BlockBufferPartition previous_block = draw_instances.blocks.count(batch) ? draw_instances.blocks.at(batch) : BlockBufferPartition{0, 0, {}};
    std::vector<DrawInstanceData> instances(count);
    for (int i = 0; i < count; i++) {
        instances[i].transform_id = draw_transform.blocks.at(batch).offset + offset + i;
        instances[i].material_id = material;
    }
    Buffer::addToBlockBuffer(draw_instances, batch, (void*)instances.data(), count, false);
    
    //: If the block is relocated, the transform ids need to be updated
    if (previous_block.size > 0 and previous_block.offset != draw_instances.blocks.at(batch).offset) {
        //: Create staging buffer to modify the data
        BufferData staging_buffer = Buffer::API::allocateBuffer(previous_block.size * sizeof(DrawInstanceData), BufferUsage(BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_TRANSFER_DST), BUFFER_MEMORY_CPU_ONLY, nullptr);
        
        //: Copy the previous block to the staging buffer
        Buffer::API::copyBuffer(draw_instances.buffer, staging_buffer, previous_block.size * sizeof(DrawInstanceData), 0, previous_block.offset * sizeof(DrawInstanceData));
        
        //: Map data from staging buffer
        void* data_;
        vmaMapMemory(api.allocator, staging_buffer.allocation, &data_);
        DrawInstanceData* data = (DrawInstanceData*)data_;
        
        //: Update values
        for (int i = 0; i < previous_block.size; i++)
            data[i].transform_id += draw_instances.blocks.at(batch).offset - previous_block.offset;
        
        //: Copy again to the GPU buffer
        Buffer::API::copyBuffer(staging_buffer, draw_instances.buffer, previous_block.size * sizeof(DrawInstanceData), draw_instances.blocks.at(batch).offset * sizeof(DrawInstanceData));
        
        //: Destroy staging buffer
        vmaUnmapMemory(api.allocator, staging_buffer.allocation);
        Buffer::API::destroyBuffer(staging_buffer);
        buffer_list.erase(staging_buffer);
    }
    
    //: Find new id
    DrawID id = draw_descriptions.size() == 0 ? 0 : draw_descriptions.rbegin()->first;
    do { id++; } while (draw_descriptions.count(id));
    
    //: Create new draw description
    draw_descriptions[id] = DrawDescription{batch, type, mesh, count, offset, draw_shaders};
    return id;
}


//---------------------------------------------------
//: Draw
//      Adds a DrawID to the render queue for draw indirect command creation
//---------------------------------------------------
void Draw::draw(DrawID id) {
    draw_queue.insert(id);
}

//---------------------------------------------------
//: Get instance data
//      Returns a pointer to the draw scene buffer for the specified draw id data
//---------------------------------------------------
DrawTransformData* Draw::getInstanceData(DrawID id) {
    //: Check that the DrawID is valid
    if (not draw_descriptions.count(id))
        log::error("Invalid Draw ID %d", id);
    
    //: Check if the buffer is mapped
    if (draw_transform_data == nullptr)
        log::error("The draw scene buffer is not mapped into memory");
    
    //: Return a pointer to the correct offset to the mapped buffer data
    auto &object = draw_descriptions.at(id);
    auto &batch = draw_transform.blocks.at(object.hash);
    return (DrawTransformData*)draw_transform_data + object.offset + batch.offset;
}

//---------------------------------------------------
//: Build draw queue
//---------------------------------------------------
ui32 calculateHash(ui32 count, ui32 offset, ui32 hash) {
    std::stringstream ss;
    ss << count;
    ss << offset;
    ss << hash;
    auto s = ss.str();
    return Math::hash(s.c_str(), s.size());
}

void Draw::buildDrawQueue() {
    //: Calculate new and removed draws
    std::set<DrawID> new_draws;
    std::set<DrawID> removed_draws{};
    std::set_difference(draw_queue.begin(), draw_queue.end(),
                        previous_draw_queue.begin(), previous_draw_queue.end(),
                        std::inserter(new_draws, new_draws.end()));
    std::set_difference(previous_draw_queue.begin(), previous_draw_queue.end(),
                        draw_queue.begin(), draw_queue.end(),
                        std::inserter(removed_draws, removed_draws.end()));
    
    std::map<ui32, DrawID> modified_batches{};
    std::set<ui32> outdated_commands{};
    
    //: New draws
    for (auto &d_id : new_draws) {
        const auto &draw = draw_descriptions.at(d_id);
        
        //: Add the new draw
        draw_batches[draw.hash].push_back({draw.count, draw.offset, 0});

        //: If this hash has not been added to the modified batches, add it
        if (not modified_batches.count(draw.hash))
            modified_batches[draw.hash] = d_id;
    }
    
    //: Modified batches
    for (auto &[hash, d_id] : modified_batches) {
        auto &batches = draw_batches.at(hash);
        
        //: Sort
        std::sort(batches.begin(), batches.end(), [](auto &a, auto &b){ return a.offset < b.offset; });
        
        //: New flat vector
        std::vector<DrawBatch> flat_batches = { batches.at(0) };
        std::set<ui32> batches_to_reupload{};
        if (batches.at(0).command_hash == 0)
            batches_to_reupload.insert(0);
        
        //: Flatten
        for (int i = 1; i < batches.size(); i++) {
            if (batches.at(i).offset == flat_batches.rbegin()->count + flat_batches.rbegin()->offset) {
                flat_batches.rbegin()->count += batches.at(i).count;
                batches_to_reupload.insert((ui32)(flat_batches.size() - 1));
                if (batches.at(i).command_hash != 0)
                    outdated_commands.insert(batches.at(i).command_hash);
            } else {
                flat_batches.push_back(batches.at(i));
                if (batches.at(i).command_hash == 0)
                    batches_to_reupload.insert((ui32)(flat_batches.size() - 1));
            }
        }
        batches = flat_batches;
        
        //: Draw commands for batches
        const auto &draw = draw_descriptions.at(d_id);
        for (auto i : batches_to_reupload) {
            auto &batch = batches.at(i);
            
            //: Calculate hashes
            batch.command_hash = calculateHash(batch.count, batch.offset, hash);
            
            //: Add draw commands
            registerDrawCommand(batch.command_hash, draw.mesh, batch.count, draw_transform.blocks.at(hash).offset + batch.offset);
            for (auto &shader : draw.shaders)
                built_draw_queue[shader].insert(batch.command_hash);
        }
    }
    
    //: Remove outdated commands
    for (auto &hash : outdated_commands) {
        draw_commands.free_blocks.push_back(draw_commands.blocks.at(hash));
        draw_commands.blocks.erase(hash);
        for (auto &[shader, queue] : built_draw_queue)
            queue.erase(hash);
    }
    if (outdated_commands.size() > 1)
        std::sort(draw_commands.free_blocks.begin(), draw_commands.free_blocks.end(), [](auto &a, auto &b){ return a.offset < b.offset; });
    
    //: TODO: Remove draws

    //TODO: VERY TEMPORARY, TEST
    log::info("TODO: NEXT");
    //Only copy dynamic and changed buffers, move it to a separate thread, syncronization?
    //It needs to take into account static and dynamic values, only reupload when changing, and tightly packing them
    //Also, the transform ids need to be changed from the instance buffer, maybe using a compute shader is a good idea
    Buffer::API::copyBuffer(draw_transform.buffer, draw_transform_gpu, draw_transform.free_blocks.rbegin()->offset * sizeof(DrawTransformData));
    
    previous_draw_queue = draw_queue;
    draw_queue.clear();
}
