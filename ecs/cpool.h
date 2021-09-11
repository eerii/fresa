//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "ecs.h"

namespace Verse
{
    struct ComponentPool {
        ui8* pool_data{ nullptr };
        size_t element_size{ 0 };
        
        ComponentPool(size_t p_size);
        
        ~ComponentPool();
        
        void* get(size_t index);
    };
}
