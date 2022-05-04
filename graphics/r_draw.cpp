//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#include "r_draw.h"

using namespace Fresa;
using namespace Graphics;

constexpr ui32 draw_pool_chunk = 256 * draw_command_size;

//---------------------------------------------------
//: Allocate draw indirect command buffer
//      Create a new draw indirect buffer expanding the previous size and, if existent, copy the previous contents
//---------------------------------------------------
DrawCommandBuffer Draw::allocateDrawCommandBuffer() {
    DrawCommandBuffer new_draw_commands;
    
    //: Expand the buffer size by one chunk
    ui32 previous_size = draw_commands.pool_size;
    new_draw_commands.pool_size = previous_size + draw_pool_chunk;
    
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
DrawCommandID Draw::registerDrawCommand() {
    //: Get id from free positions
    DrawCommandID id{draw_commands.free_positions.begin()->value};
    
    //: Check if the id is out of bounds and recreate the command buffer
    if ((id.value + 1) * draw_command_size > draw_commands.pool_size) {
        draw_commands = allocateDrawCommandBuffer();
        if (draw_commands.pool_size > draw_pool_chunk)
            log::warn("Growing draw command buffer more than one pool chunk, consider increasing it for efficiency");
    }
    
    //: Update free positions
    if (id.value == draw_commands.free_positions.rbegin()->value)
        draw_commands.free_positions.rbegin()->value++;
    else
        draw_commands.free_positions.erase(draw_commands.free_positions.begin());
    
    //: Create indirect draw command
    std::vector<VkDrawIndexedIndirectCommand> cmd(1);
    cmd.at(0).indexCount = meshes.paddings.begin()->second.index_size / meshes.paddings.begin()->second.index_bytes;
    cmd.at(0).instanceCount = 1000;
    cmd.at(0).firstIndex = 0;
    cmd.at(0).vertexOffset = 0; //
    cmd.at(0).firstInstance = 0; //instance.instance_count * i++;
    
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
