//* ecs
//      entity component system implementation
//      !WIP ...
#pragma once

#include "std_types.h"
#include "type_name.h"

namespace fresa::ecs
{
    //* entity id
    //      this is a numerical handle that references an entity
    //      it is composed of a version and an index, being the index the first 16 bits and the version the last 16 bits
    //      the index is the entity handle, while the version exists to reuse deleted entity ids
    using EntityID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Bitwise, strong::BitwiseWith<int>>;
    namespace detail
    {
        using EntityIndex = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<EntityID>>;
        using EntityVersion = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<EntityID>>;

        constexpr EntityID createEntityID(EntityIndex index, EntityVersion version) noexcept {
            return (EntityID(index) << 16) | EntityID(version);
        }
        [[nodiscard]] constexpr EntityIndex getEntityIndex(EntityID id) noexcept { return EntityIndex((id << 16).value); }
        [[nodiscard]] constexpr EntityVersion getEntityVersion(EntityID id) noexcept { return EntityVersion(id.value); }
    };
    constexpr EntityID invalid_entity = detail::createEntityID(-1, 0);

    //* component pool
    //      ...
    //- TODO: improve this basic allocator using blocks and chunks
    template<typename T>
    struct ComponentPool {
        //: default constructor, no copy or move, destroy when out of scope
        ComponentPool() = default;
        ComponentPool(const ComponentPool&) = delete;
        ComponentPool& operator=(const ComponentPool&) = delete;
        ComponentPool(ComponentPool&&) = delete;
        ComponentPool& operator=(ComponentPool&&) = delete;
        ~ComponentPool() noexcept { destroy(); }

        T* data;
        ui32 size;
        std::allocator<T> allocator;

        constexpr T* create(std::size_t s) noexcept {
            data = allocator.allocate(s);
            size = s;
            return data;
        }

        constexpr T* resize(std::size_t s) noexcept {
            if (s > size) {
                T* new_data = allocator.allocate(s);
                std::copy(data, data + size, new_data);
                allocator.deallocate(data, size);
                data = new_data;
                size = s;
            }
            return data;
        }

        constexpr void destroy() noexcept {
            if (data) {
                allocator.deallocate(data, size);
                data = nullptr;
                size = 0;
            }
        }
    };

    namespace detail
    {
        template <typename C>
        constexpr auto& getPool() {
            constexpr TypeHash t = type_hash<C>();

            //- ...
        }
    }

    //* scene
    //      ...
    struct Scene {
        
    };
}