//* ecs
//      entity component system implementation
//      !WIP ...
#pragma once

#include "fresa_types.h"
#include "strong_types.h"

namespace fresa::ecs
{
    //* entity id
    //      this is a numerical handle that references an entity
    using EntityID = strong::Type<ui32, decltype([]{}), strong::Regular>;
}