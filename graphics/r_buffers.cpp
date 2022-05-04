//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_buffers.h"

using namespace Fresa;
using namespace Graphics;

constexpr ui32 mesh_pool_chunk = 256000; // 256kb * 2

//---------------------------------------------------
//: Allocate mesh buffer
//      Create a new mesh buffer expanding the previous size and, if existent, copy the previous contents
//---------------------------------------------------
MeshBuffers Buffer::allocateMeshBuffer() {
    MeshBuffers new_meshes;
    
    //: Expand the buffer size by one chunk
    ui32 previous_size = meshes.pool_size;
    new_meshes.pool_size = previous_size + mesh_pool_chunk;
    
    //: Allocate the vertex and index buffers
    new_meshes.vertex_buffer = Common::allocateBuffer(new_meshes.pool_size, BufferUsage(BUFFER_USAGE_VERTEX | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), BUFFER_MEMORY_GPU_ONLY);
    new_meshes.index_buffer = Common::allocateBuffer(new_meshes.pool_size, BufferUsage(BUFFER_USAGE_INDEX | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), BUFFER_MEMORY_GPU_ONLY);
    
    //: If it has contents, copy them to the new buffers
    //      Finds the last element (the one with the highest offset) for the vertex and index buffers
    //      Then copies the buffer from the beginning to the offset + size of the last element
    //      Should be reasonably efficient providing the elements are tightly packed
    if (meshes.paddings.size() > 0) {
        //: Vertex buffer
        auto last_vertex = std::max_element(meshes.paddings.begin(), meshes.paddings.end(),
                                            [](auto &a, auto &b)->bool{ return a.second.vertex_offset < b.second.vertex_offset; } );
        Common::copyBuffer(meshes.vertex_buffer, new_meshes.vertex_buffer, last_vertex->second.vertex_offset + last_vertex->second.vertex_size);
        
        //: Index buffer
        auto last_index = std::max_element(meshes.paddings.begin(), meshes.paddings.end(),
                                           [](auto &a, auto &b)->bool{ return a.second.index_offset < b.second.index_offset; } );
        Common::copyBuffer(meshes.index_buffer, new_meshes.index_buffer, last_index->second.index_offset + last_index->second.index_size);
        
        //: Copy mesh paddings
        new_meshes.paddings = meshes.paddings;
    }
    
    return new_meshes;
}

//---------------------------------------------------
//: Register mesh
//      Adds the mesh data to the mesh buffers with the appropiate offset and padding
//      Also checks if the buffer is too small and extends it otherwise
//      Returns a MeshID for referencing during draw
//---------------------------------------------------
MeshID Buffer::registerMeshInternal(void *vertices, void *indices, ui32 vertices_size, ui32 indices_size, ui8 index_bytes) {
    MeshID id{0};
    ui32 vertices_offset = 0;
    ui32 indices_offset = 0;
    
    //: Error checking
    if (vertices_size > mesh_pool_chunk)
        log::error("Adding vertices to the mesh buffer with a size of %d but the max pool chunk is %d", vertices_size, mesh_pool_chunk);
    if (indices_size > mesh_pool_chunk)
        log::error("Adding indices to the mesh buffer with a size of %d but the max pool chunk is %d", indices_size, mesh_pool_chunk);
    
    //: If there are more meshes
    if (meshes.paddings.size() > 0) {
        //: Find new mesh id
        id = meshes.paddings.rbegin()->first;
        do { id = id + 1; } while (meshes.paddings.count(id));
        
        //: Get last elements so we can use the offset
        auto last_vertex = std::max_element(meshes.paddings.begin(), meshes.paddings.end(),
                                            [](auto &a, auto &b)->bool{ return a.second.vertex_offset < b.second.vertex_offset; } );
        auto last_index = std::max_element(meshes.paddings.begin(), meshes.paddings.end(),
                                           [](auto &a, auto &b)->bool{ return a.second.index_offset < b.second.index_offset; } );
        vertices_offset = last_vertex->second.vertex_offset + last_vertex->second.vertex_size;
        indices_offset = last_index->second.index_offset + last_index->second.index_size;
    }
    
    //: Grow mesh buffer if it is full
    if (vertices_offset + vertices_size > meshes.pool_size or indices_offset + indices_size > meshes.pool_size) {
        meshes = allocateMeshBuffer();
        if (meshes.pool_size > mesh_pool_chunk)
            log::warn("Growing mesh buffer more than one pool chunk, consider increasing it for efficiency");
    }
    
    //: Update buffers
    Common::updateBuffer(meshes.vertex_buffer, vertices, vertices_size, vertices_offset);
    Common::updateBuffer(meshes.index_buffer, indices, indices_size, indices_offset);
    
    //: Check index bytes
    if (not (index_bytes == 2 or index_bytes == 4))
        log::error("Index buffers support ui16 (2 bytes) and ui32 (4 bytes) formats, but you loaded one index with %d bytes", index_bytes);
    
    //: Update paddings
    meshes.paddings[id] = MeshPadding{vertices_offset, vertices_size, indices_offset, indices_size, index_bytes};
    
    return id;
}

//---------------------------------------------------
//: Register uniform buffer
//---------------------------------------------------
UniformBufferID Buffer::registerUniformBuffer(ui32 size) {
    //: Find new id
    UniformBufferID id = uniform_buffers.size() == 0 ? UniformBufferID{} : uniform_buffers.end()->first;
    do { id.value++; } while (id == no_uniform_buffer or uniform_buffers.count(UniformBufferID{id}));
    
    //: Allocate new buffer
    uniform_buffers[id] = Common::allocateBuffer(size, BUFFER_USAGE_UNIFORM, BUFFER_MEMORY_BOTH);
    
    return id;
}

//---------------------------------------------------
//: Register storage buffer
//      Adds a storage buffer to the list and returns an id for indexing
//      If the buffer name is part of the key list and the buffer is allocated, return the id
//---------------------------------------------------
StorageBufferID Buffer::registerStorageBuffer(str name, ui32 size) {
    //: If the name is part of the key list and it is already allocated, return the previous id
    if (reserved_storage_buffers.count(name) and reserved_storage_buffers.at(name) != no_storage_buffer)
        return reserved_storage_buffers.at(name);
    
    //: Find new id
    StorageBufferID id = storage_buffers.size() == 0 ? StorageBufferID{} : storage_buffers.end()->first;
    do { id.value++; } while (id == no_storage_buffer or storage_buffers.count(StorageBufferID{id}));
    
    //: Save the id to the key list if it was part of it but not allocated
    if (reserved_storage_buffers.count(name))
        reserved_storage_buffers.at(name) = id;
    
    //: Allocate new buffer
    storage_buffers[id] = Common::allocateBuffer(size, BUFFER_USAGE_STORAGE, BUFFER_MEMORY_BOTH);
    
    return id;
}
