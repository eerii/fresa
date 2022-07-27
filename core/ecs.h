//* ecs
//      entity component system implementation
#pragma once

#include "std_types.h"
#include "type_name.h"
#include "log.h"
#include <deque>

namespace fresa::ecs
{
    //* index-version id
    //      this is a numerical handle composed of a version and an index, being the version the first 16 bits and the index the last 16 bits
    //      the index is the entity handle, while the version exists to reuse deleted entity ids
    //      the id meant to be aliased for the types that require it, such as the entity and sparse set
    namespace detail 
    {
        using ID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Bitwise, strong::BitwiseWith<int>>;
    }
    using Index = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<detail::ID>, strong::Hashable>;
    using Version = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<detail::ID>, strong::Ordered>;

    [[nodiscard]] constexpr Index index(detail::ID id) noexcept { return Index(id.value); }
    [[nodiscard]] constexpr Version version(detail::ID id) noexcept { return Version((id >> 16).value); }
    [[nodiscard]] constexpr detail::ID id(Index i, Version v) noexcept { return (detail::ID(v) << 16) | detail::ID(i); }

    constexpr detail::ID invalid_id = id(-1, 0);

    //: alias for entities
    using EntityID = detail::ID;

    //---

    //* component pool
    //      a component pool is an allocator that stores components of a certain type
    //      the pool base is used to store references to the component pool in a hashed type map for later access
    //      the pool is a sparse set that stores the components using a dense array

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
        //: alias for sparse set
        using SparseID = detail::ID;

        //: data
        std::unordered_map<Index, std::array<SparseID, engine_config.ecs_page_size()>> sparse;
        std::vector<T> dense;

        //: get sparse
        //      gets the entity index and sees if it is included in the sparse array
        [[nodiscard]] SparseID* sparse_at(const EntityID entity) {
            const auto pos = index(entity).value;
            const auto page = pos / engine_config.ecs_page_size();
            return sparse.contains(page) ? &(sparse.at(page).at(pos % engine_config.ecs_page_size())) : nullptr;
        }

        //: is valid
        //      checks if an sparse id handle points to something and has a propper version
        [[nodiscard]] bool valid(const SparseID* sid, const Version v) const {
            return sid != nullptr and *sid != invalid_id and version(*sid) == v;
        }

        //: contains entity
        //      checks if the entity is included in the sparse array, also verifying the version
        [[nodiscard]] bool contains(const EntityID entity) const {
            return valid(sparse_at(entity), version(entity));
        }

        //: add
        //      adds an entity to the sparse array, if there is an entity with a lower version it is updated
        //      if the entity has the same or higher version, an error is thrown
        void add(const EntityID entity, T&& value) {
            const auto pos = index(entity).value;
            const auto page = pos / engine_config.ecs_page_size();

            if (not sparse.contains(page))
                sparse[page].fill(invalid_id);

            auto& element = *sparse_at(pos);
            if (element == invalid_id) {
                element = id(dense.size(), version(entity));
                dense.emplace_back(std::move(value));
            } else if (version(entity) > version(element)) {
                element = id(index(element), version(entity));
                dense.at(index(element).value) = std::move(value);
            } else {
                log::error("entity {} with version {} already exists in sparse set", entity.value, version(entity).value);
            }
        }

        //: get
        //      returns a pointer to the entity value from the dense array if it exists, if not it returns nullptr
        [[nodiscard]] const T* get(const EntityID entity) {
            const auto sid = sparse_at(entity);
            return valid(sid, version(entity)) ? &dense.at(index(*sid).value) : nullptr;
        }

        //: remove
        //      removes an entity if it exists, otherwise it does nothing
        //      it swaps the last element with the removed element from both the sparse and dense arrays
        //      this function uses find if to find the last entity, however, it might be faster to use a dedicated extra array that holds
        //      the references to the entities from each spot of the dense array
        void remove(const EntityID entity) {
            const auto sid = sparse_at(entity);
            if (not valid(sid, version(entity))) return;

            const auto last_dense = dense.size() - 1;
            SparseID* last_sparse = nullptr;
            for (auto &[key, page] : sparse) {
                last_sparse = std::find_if(page.begin(), page.end(), [&](const auto &e) { return index(e) == last_dense; });
                if (last_sparse != page.end()) break;
            };

            auto removed_element = sparse_at(index(entity).value);
            std::swap(*last_sparse, *removed_element);
            std::swap(dense.back(), dense.at(index(*sid).value));

            *removed_element = invalid_id;
            dense.pop_back();
        }

        //: clear
        void clear() {
            sparse.clear();
            dense.clear();
        }

        //: size and extent
        [[nodiscard]] std::size_t size() const { return dense.size(); }
        [[nodiscard]] std::size_t extent() const { return sparse.size() * engine_config.ecs_page_size(); }

        //: iterators
        [[nodiscard]] auto begin() const noexcept { return dense.begin(); }
        [[nodiscard]] auto end() const noexcept { return dense.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return dense.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return dense.cend(); }
        [[nodiscard]] auto rbegin() const noexcept { return dense.rbegin(); }
        [[nodiscard]] auto rend() const noexcept { return dense.rend(); }
        [[nodiscard]] auto crbegin() const noexcept { return dense.crbegin(); }
        [[nodiscard]] auto crend() const noexcept { return dense.crend(); }
    };

    //---

    //* scene
    //      ...
    struct Scene {
        //* component pool
        //      searchs the hash map for the specified component type
        //      if none is found, create it. this operation is thread safe

        //: hash map of component pools, using the component type as a key
        std::unordered_map<TypeHash, std::unique_ptr<ComponentPoolBase>> component_pools;

        //: this mutex prevents the creation of multiple component pools of the same type
        std::mutex component_pool_create_mutex;

        //: get component pool
        template <typename C>
        auto& cpool() {
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

        // ---

        //* entities
        //      ...

        std::deque<EntityID> free_entities = {ecs::id(0, 0)};

        //: add entity
        template <typename ... C>
        const EntityID add(C&& ... components) {
            const auto entity = free_entities.front();
            if (free_entities.size() > 1) free_entities.pop_front();
            else free_entities.back() = entity.value + 1;
            (cpool<C>().add(entity, std::forward<C>(components)), ...);
            return entity;
        }
    };
}