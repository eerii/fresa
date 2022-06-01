//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_draw.h"
#include "r_api.h"
#include "f_math.h"
#include <sstream>

using namespace Fresa;
using namespace Graphics;

//---------------------------------------------------
//: Register draw command
//---------------------------------------------------
DrawCommandID Draw::registerDrawCommand(DrawQueueObject draw) {
    //: Check if block buffer is available
    if (draw_commands.buffer.buffer == no_buffer)
        draw_commands = Buffer::createBlockBuffer(256, sizeof(IDrawIndexedIndirectCommand), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    //: Create indirect draw command
    //TODO: (3) Pack multiple commands together
    
    IDrawIndexedIndirectCommand cmd;
    cmd.indexCount = meshes.index.blocks.at(draw.mesh).size;
    cmd.instanceCount = draw.instance_count;
    cmd.firstIndex = meshes.index.blocks.at(draw.mesh).offset;
    cmd.vertexOffset = meshes.vertex.blocks.at(draw.mesh).offset;
    cmd.firstInstance = draw.instance_offset;
    
    //: Add to block buffer
    return Buffer::addToBlockBuffer(draw_commands, 0, (void*)&cmd, 1);
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
    //      Iit takes into account the mesh (and possible more things in the future), and separates static and dynamic objects
    std::stringstream ss;
    if (type & DRAW_STATIC)
        ss << DRAW_STATIC;
    if (type & DRAW_DYNAMIC)
        ss << DRAW_DYNAMIC;
    ss << mesh;
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
    
    //: Find if there is a matching draw description (same shaders and batch id)
    /*auto it = std::find_if(draw_descriptions.begin(), draw_descriptions.end(), [&](const auto &d){
        return draw_shaders == d.second.shaders and batch == d.second.hash;
    });
    
    
    //: There is already draw matching the description
    if (it != draw_descriptions.end()) {
        auto &draw = it->second;
        
        //: Check if the descriptions are propperly packed
        if (offset != draw.offset + draw.count)
            log::error("These draw descriptions are not tightly packed");
        
        //: Add instance count
        draw.count += count;
        
        return it->first;
    }
    
    //: This needs a new description
    else {*/
        //: Find new id
        DrawID id = draw_descriptions.size() == 0 ? 0 : draw_descriptions.rbegin()->first;
        do { id++; } while (draw_descriptions.count(id));
        
        //: Create new draw description
        draw_descriptions[id] = DrawDescription{batch, type, mesh, material, count, offset, draw_shaders};
        return id;
    //}
}


//---------------------------------------------------
//: Draw
//      Adds a DrawID to the render queue for draw indirect command creation
//---------------------------------------------------
void Draw::draw(DrawID id) {
    //TODO: (1) See how to organize draw ids and batches
    
    //: Check if the DrawID is valid
    if (not draw_descriptions.count(id))
        log::error("Using invalid DrawID (%d)", id);
    auto &object = draw_descriptions.at(id);
    
    for (auto &shader : object.shaders) {
        auto it = std::find_if(draw_queue.begin(), draw_queue.end(), [&](const auto &d){
            return d.shader == shader and d.hash == object.hash;
        });
        //: Add to draw queue (first item)
        if (it == draw_queue.end()) {
            DrawQueueObject new_draw{};
            new_draw.shader = shader;
            new_draw.hash = object.hash;
            new_draw.command = no_draw_command;
            new_draw.mesh = object.mesh;
            new_draw.instance_count = object.count;
            new_draw.instance_offset = draw_transform.blocks.at(object.hash).offset + object.offset; //TODO: Check if they are tightly packed
            draw_queue.push_back(new_draw);
        }
        //: Add to draw queue (next items)
        else {
            it->instance_count += object.count;
        }
    }
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
void Draw::buildDrawQueue() {
    //TODO: (2) Fix this once the draw queue is draw ids, also see how to transfer the draw queue to the GPU
    //          It needs to take into account static and dynamic values, only reupload when changing, and tightly packing them
    //          Also, the transform ids need to be changed from the instance buffer, maybe using a compute shader is a good idea
    
    //: New draws
    ui32 new_draw_count = 0;
    for (auto &d : draw_queue) {
        auto it = std::find_if(previous_draw_queue.begin(), previous_draw_queue.end(), [&d](const auto &i){
            return i.shader == d.shader and i.hash == d.hash and i.instance_count == d.instance_count;
        });
        if (it == previous_draw_queue.end()) {
            if (d.command == no_draw_command)
                d.command = registerDrawCommand(d);
            built_draw_queue[d.shader].push_back(d.command);
            new_draw_count++;
        }
    }
    
    //: Removed draws
    if (previous_draw_queue.size() + new_draw_count > draw_queue.size()) {
        for (auto &d : previous_draw_queue) {
            auto it = std::find_if(draw_queue.begin(), draw_queue.end(), [&d](const auto &i){
                return i.shader == d.shader and i.hash == d.hash and i.instance_count == d.instance_count;
            });
            if (it == draw_queue.end()) {
                Buffer::removeFromBlockBuffer(draw_commands, 0, d.command);
                auto it_ = std::find_if(built_draw_queue.at(d.shader).begin(), built_draw_queue.at(d.shader).end(),
                                        [&d](const auto &i){ return i == d.command; });
                built_draw_queue.at(d.shader).erase(it_);
                d.command = no_draw_command;
                if (built_draw_queue.at(d.shader).size() == 0)
                    built_draw_queue.erase(d.shader);
            }
        }
    }

    //TODO: VERY TEMPORARY, TEST
    //Only copy dynamic and changed buffers, move it to a separate thread, syncronization?
    Buffer::API::copyBuffer(draw_transform.buffer, draw_transform_gpu, draw_transform.free_blocks.rbegin()->offset * sizeof(DrawTransformData));
    
    previous_draw_queue = draw_queue;
    draw_queue.clear();
}
