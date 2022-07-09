//* ecs
//      entity component system implementation
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

    //---

    //* component pool
    //      a component pool is an allocator that stores components of a certain type
    //      the pool base is used to store references to the component pool in a hashed type map for later access
    //      - improve this basic allocator using blocks and chunks

    //: base component pool
    //      default constructor, no copy or move, destroyed when out of scope
    struct ComponentPoolBase {
        ComponentPoolBase() = default;
        ComponentPoolBase(const ComponentPoolBase&) = delete;
        ComponentPoolBase& operator=(const ComponentPoolBase&) = delete;
        ComponentPoolBase(ComponentPoolBase&&) = delete;
        ComponentPoolBase& operator=(ComponentPoolBase&&) = delete;
        constexpr virtual ~ComponentPoolBase() = default;
    };

    //: typed component pool
    template<typename T>
    struct ComponentPool : ComponentPoolBase {
        //: clear on destruction
        constexpr ~ComponentPool() noexcept { clear(); }

        //: data and size of the allocation
        T* data;
        ui32 size;

        //: allocator
        std::allocator<T> allocator;

        //: create the pool
        //      controlled by the once flag to avoid multiple calls to the allocator, use resize for reallocation
        std::once_flag create_only_once;
        constexpr T* create(ui64 s) noexcept {
            std::call_once(create_only_once, [this, s]{
                data = allocator.allocate(s);
                size = s;
            });
            return data;
        }

        //: resize the pool
        constexpr T* resize(ui64 s) noexcept {
            if (s > size) {
                T* new_data = allocator.allocate(s);
                std::copy(data, data + size, new_data);
                allocator.deallocate(data, size);
                data = new_data;
                size = s;
            }
            return data;
        }

        //: deallocate the pool data
        constexpr void clear() noexcept {
            if (data) {
                allocator.deallocate(data, size);
                data = nullptr;
                size = 0;
            }
        }
    };

    namespace detail
    {
        //: hash map of component pools, using the component type as a key
        std::unordered_map<TypeHash, std::unique_ptr<ComponentPoolBase>> component_pools;

        //: this mutex prevents the creation of multiple component pools of the same type
        std::mutex component_pool_create_mutex;

        //* get component pool
        //      searchs the hash map for the specified component type
        //      if none is found, create it. this operation is thread safe
        template <typename C>
        auto& getPool() {
            constexpr TypeHash t = type_hash<C>();
            std::lock_guard<std::mutex> lock(component_pool_create_mutex);

            //: find the pool using the type hash
            auto it = component_pools.find(t);

            //: there is no pool, create it
            if (it == component_pools.end()) {
                auto pool = std::make_unique<ComponentPool<C>>();
                it = component_pools.emplace(t, std::move(pool)).first;
            }

            //: return a reference to the pool
            return (ComponentPool<C>&)(*it->second);
        }
    }

    //---

    //* scene
    //      ...
    struct Scene {
        
    };
}