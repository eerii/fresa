//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "ecs.h"

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
