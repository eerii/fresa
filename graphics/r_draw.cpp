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

//---------------------------------------------------
//: Register draw command
//---------------------------------------------------
DrawCommandID Draw::registerDrawCommand(DrawQueueObject draw) {
    //: Check if block buffer is available
    if (draw_commands.buffer.buffer == no_buffer)
        draw_commands = Buffer::createBlockBuffer(256, draw_command_size, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_INDIRECT), BUFFER_MEMORY_GPU_ONLY);
    
    //: Create indirect draw command
    VkDrawIndexedIndirectCommand cmd;
    cmd.indexCount = meshes.index.blocks.at(draw.mesh.index).size;
    cmd.instanceCount = draw.instance_count;
    cmd.firstIndex = meshes.index.blocks.at(draw.mesh.index).offset;
    cmd.vertexOffset = meshes.vertex.blocks.at(draw.mesh.vertex).offset;
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
    MeshID id{};
    
    //: Check if block buffers are created, if not, allocate them
    if (meshes.vertex.buffer.buffer == no_buffer)
        meshes.vertex = Buffer::createBlockBuffer(4092, vertex_bytes, BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_VERTEX), BUFFER_MEMORY_GPU_ONLY);
    if (meshes.index.buffer.buffer == no_buffer)
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
    
    return id;
}

//---------------------------------------------------
//: Register draw id
//      Creates a new draw description using some parameters
//      Also creates a draw batch by hashing those parameters to group compatible draws
//      Adds it to the batches buffer and calculates the batch offset
//---------------------------------------------------
DrawID Draw::registerDrawID(MeshID mesh, DrawBatchType type, ui32 count) {
    //: Find new id
    DrawID id = draw_descriptions.size() == 0 ? DrawID{} : draw_descriptions.end()->first;
    do { id.value++; } while (draw_descriptions.count(id));
    
    //: Create the draw object
    DrawDescription draw{};
    draw.mesh = mesh;
    draw.type = type;
    draw.count = count;
    
    //: Calculate the draw batch hast
    //      For single objects, the batch id is all the same. For instances it takes into account the mesh (and possible more things in the future)
    //      They are also separated by static and dynamic objects
    std::stringstream ss;
    if (type & DRAW_SINGLE_OBJECT)
        ss << DRAW_SINGLE_OBJECT;
    if (type & DRAW_STATIC)
        ss << DRAW_STATIC;
    if (type & DRAW_DYNAMIC)
        ss << DRAW_DYNAMIC;
    if (type & DRAW_INSTANCES) {
        ss << draw.mesh.vertex;
        ss << draw.mesh.index;
    }
    auto s = ss.str();
    draw.batch = DrawBatchID{Math::hash(s.c_str(), s.size())};
    
    //: Make sure block buffer is created
    if (draw_instances.buffer.buffer == no_buffer)
        draw_instances = Buffer::createBlockBuffer(1024, sizeof(DrawInstanceData), BufferUsage(BUFFER_USAGE_TRANSFER_DST | BUFFER_USAGE_TRANSFER_SRC | BUFFER_USAGE_STORAGE), BUFFER_MEMORY_BOTH, [](BufferData &buffer){
            //: Link new buffer handle to shaders that use it
            for (const auto &list : shaders.list) {
                for (const auto &[id, shader] : list) {
                    [id=id, shader=shader](){
                    for (const auto &d : shader.descriptors) {
                        for (const auto &res : d.resources) {
                            if (res.type != DESCRIPTOR_STORAGE)
                                continue;
                            auto r_id = std::get<StorageBufferID>(res.id);
                            auto it = std::find_if(reserved_buffers.begin(), reserved_buffers.end(),
                                                   [&r_id](auto &b){ return b.id == r_id; });
                            if (it != reserved_buffers.end()) {
                                Shader::API::linkDescriptorResources(id);
                                return;
                            }
                        }
                    }}();
                }
            }
            
            //: Map the buffer to the cpu
            vmaMapMemory(api.allocator, buffer.allocation, &draw_instance_data);
        });
    
    //: Add to block buffer
    draw.offset = Buffer::addToBlockBuffer(draw_instances, draw.batch.value, nullptr, count, false);
    
    //: Add to scene objects and to the recreation queue
    draw_descriptions[id] = draw;
    
    return id;
}


//---------------------------------------------------
//: Draw
//      Adds a DrawID to the specified shader's render queue for draw indirect command creation
//      Count adds that number of contiguous draw calls to the instance count
//---------------------------------------------------
void Draw::draw(ShaderID shader, DrawID id) {
    //: Check if the DrawID is valid
    if (not draw_descriptions.count(id))
        log::error("Using invalid DrawID (%d)", id.value);
    auto &object = draw_descriptions.at(id);
    
    //: Add to draw queue (first item)
    auto it = std::find_if(draw_queue.begin(), draw_queue.end(), [&](const auto &d){
        return d.shader == shader and d.batch == object.batch;
    });
    if (it == draw_queue.end()) {
        DrawQueueObject new_draw{};
        new_draw.shader = shader;
        new_draw.batch = object.batch;
        new_draw.command = no_draw_command;
        new_draw.mesh = object.mesh;
        new_draw.instance_count = object.count;
        new_draw.instance_offset = draw_instances.blocks.at(object.batch.value).offset; //TODO: Check if they are tightly packed
        draw_queue.push_back(new_draw);
    }
    //: Add to draw queue (next items)
    else {
        it->instance_count += object.count;
    }
}

//---------------------------------------------------
//: Get instance data
//      Returns a pointer to the draw scene buffer for the specified draw id data
//---------------------------------------------------
DrawInstanceData* Draw::getInstanceData(DrawID id) {
    //: Check that the DrawID is valid
    if (not draw_descriptions.count(id))
        log::error("Invalid Draw ID %d", id.value);
    
    //: Check if the buffer is mapped
    if (draw_instance_data == nullptr)
        log::error("The draw scene buffer is not mapped into memory");
    
    //: Return a pointer to the correct offset to the mapped buffer data
    auto &object = draw_descriptions.at(id);
    auto &batch = draw_instances.blocks.at(object.batch.value);
    return (DrawInstanceData*)draw_instance_data + object.offset + batch.offset;
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
                return i.shader == d.shader and i.batch == d.batch and i.instance_count == d.instance_count;
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
    
    previous_draw_queue = draw_queue;
    draw_queue.clear();
}
