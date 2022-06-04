//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "ecs.h"

//---Component pool---
//      Somewhat inefficient component allocator pool, it can be improved by using sparse sets, but it is fine for now

namespace Fresa
{
    struct ComponentPool {
        ui8* pool_data{ nullptr };
        size_t element_size{ 0 };
        
        ComponentPool(size_t p_size);
        
        ~ComponentPool();
        
        void* get(size_t index);
    };
}
