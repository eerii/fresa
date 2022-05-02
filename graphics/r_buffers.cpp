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

/*
 meshes = Buffer::allocateMeshBuffer();
 std::vector<ui32> test(100);
 for (auto &t : test)
     t = rand() % 200;
 Common::updateBuffer(meshes.vertex_buffer, (void*)test.data(), (ui32)(test.size() * sizeof(ui32)));
 meshes.paddings[MeshID{1}] = MeshPadding{0, 100 * sizeof(ui32), 0, 1};
 meshes = Buffer::allocateMeshBuffer();
 */
