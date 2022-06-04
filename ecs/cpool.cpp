//: fresa by jose pazos perez, licensed under GPLv3
#include "cpool.h"

using namespace Fresa;

ComponentPool::ComponentPool(size_t p_size) {
    element_size = p_size;
    pool_data = new ui8[element_size * MAX_ENTITIES];
}

ComponentPool::~ComponentPool() {
    delete[] pool_data;
}

void* ComponentPool::get(size_t index) {
    return pool_data + index * element_size;
}
