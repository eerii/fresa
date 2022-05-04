//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "r_buffers.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Definitions
    //---------------------------------------------------
    
    //: Indirect draw command buffer id
    using DrawCommandID = FresaType<ui16, struct DrawCommandTag>;
    inline DrawCommandID no_draw_buffer{USHRT_MAX};
    
    //: Size of the draw indirect commands
    constexpr ui32 draw_command_size = IF_VULKAN(sizeof(VkDrawIndexedIndirectCommand)) IF_OPENGL(1);
    
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Buffer holding the indirect draw commands and the offset information
    inline struct DrawCommandBuffer {
        BufferData buffer;
        std::vector<DrawCommandID> free_positions = {DrawCommandID{0}};
        ui32 pool_size = 0;
    } draw_commands;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    
    namespace Draw {
        //: Allocate draw command buffer
        DrawCommandBuffer allocateDrawCommandBuffer();
        
        //: Register draw command
        DrawCommandID registerDrawCommand();
        
        //: Remove draw command
        void removeDrawCommand(DrawCommandID id);
    }
    
    //---------------------------------------------------
    //: API dependent systems
    //      They are not implemented in this file, instead you can find them in each API code
    //---------------------------------------------------
    
    namespace Common {
        
    }
}
