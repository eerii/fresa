//* ecs
//      entity component system implementation
#pragma once

#include "std_types.h"
#include "type_name.h"
#include "log.h"

namespace fresa::ecs
{
    //* entity id
    //      this is a numerical handle that references an entity
    //      it is composed of a version and an index, being the version the first 16 bits and the index the last 16 bits
    //      the index is the entity handle, while the version exists to reuse deleted entity ids
    using EntityID = strong::Type<ui32, decltype([]{}), strong::Regular, strong::Bitwise, strong::BitwiseWith<int>>;
    namespace detail
    {
        using EntityIndex = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<EntityID>, strong::Hashable>;
        using EntityVersion = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<EntityID>, strong::Ordered>;

        [[nodiscard]] constexpr EntityIndex getIndex(EntityID id) noexcept { return EntityIndex(id.value); }
        [[nodiscard]] constexpr EntityVersion getVersion(EntityID id) noexcept { return EntityVersion((id >> 16).value); }
    };
    constexpr EntityID id(detail::EntityIndex index, detail::EntityVersion version) noexcept {
        return (EntityID(version) << 16) | EntityID(index);
    }
    constexpr EntityID invalid_entity = id(-1, 0);

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
        //: sparse id
        //      the sparse array contains objects that, much like entity ids, contain a version and an index
        //      the naming distinction is to avoid confusion with entities, since these represent the positions on the dense array
        //      functionally it behaves the same
        using SparseID = EntityID;
        using SparseIndex = detail::EntityIndex;

        //: data
        std::unordered_map<SparseIndex, std::array<SparseID, engine_config.ecs_page_size()>> sparse;
        std::vector<T> dense;

        //: get sparse
        //      gets the entity index and sees if it is included in the sparse array
        [[nodiscard]] SparseID SID(const EntityID entity) const {
            //- make pointer
            const auto pos = detail::getIndex(entity).value;
            const auto page = pos / engine_config.ecs_page_size();
            const auto page_pos = pos % engine_config.ecs_page_size();
            if (not sparse.contains(page))
                return invalid_entity;
            return sparse.at(page).at(page_pos);
        }

        //: is valid
        //      checks if an sparse id handle points to something and has a propper version
        [[nodiscard]] bool valid(const SparseID sid, const detail::EntityVersion version) const {
            return sid != invalid_entity and detail::getVersion(sid) == version;
        }

        //: contains entity
        //      checks if the entity is included in the sparse array, also verifying the version
        [[nodiscard]] bool contains(const EntityID entity) const {
            return valid(SID(entity), detail::getVersion(entity));
        }

        //: add
        //      adds an entity to the sparse array, if there is an entity with a lower version it is updated
        //      if the entity has the same or higher version, an error is thrown
        void add(const EntityID entity, T&& value) {
            //- use freed list
            const auto pos = detail::getIndex(entity).value;
            const auto page = pos / engine_config.ecs_page_size();

            if (not sparse.contains(page))
                sparse[page].fill(invalid_entity);
            auto &element = sparse.at(pos / engine_config.ecs_page_size()).at(pos % engine_config.ecs_page_size());

            if (element == invalid_entity) {
                element = id(dense.size(), detail::getVersion(entity));
                dense.emplace_back(std::move(value));
            } else if (detail::getVersion(entity) > detail::getVersion(element)) {
                element = id(detail::getIndex(element), detail::getVersion(entity));
                dense.at(detail::getIndex(element).value) = std::move(value);
            } else {
                log::error("entity {} with version {} already exists in sparse set", entity.value, detail::getVersion(entity).value);
            }
        }

        //: get
        //      returns a pointer to the entity value from the dense array if it exists, if not it returns nullptr
        [[nodiscard]] const T* get(const EntityID entity) const {
            const auto sid = SID(entity);
            return valid(sid, detail::getVersion(entity)) ? &dense.at(detail::getIndex(sid).value) : nullptr;
        }

        //: remove
        //      removes an entity if it exists, otherwise it does nothing
        //      it swaps the last element with the removed element from both the sparse and dense arrays
        //      this function uses find if to find the last entity, however, it might be faster to use a dedicated extra array that holds
        //      the references to the entities from each spot of the dense array
        void remove(const EntityID entity) {
            const auto sid = SID(entity);
            if (not valid(sid, detail::getVersion(entity))) return;

            const auto last_dense = dense.size() - 1;
            SparseID* last_sparse = nullptr;
            for (auto &[key, page] : sparse) {
                last_sparse = std::find_if(page.begin(), page.end(), [&](const auto &e) { return detail::getIndex(e) == last_dense; });
                if (last_sparse != page.end()) break;
            };
            auto* removed_element = &sparse.at(detail::getIndex(entity).value / engine_config.ecs_page_size()).at(detail::getIndex(entity).value % engine_config.ecs_page_size());
            std::swap(*last_sparse, *removed_element);
            std::swap(dense.back(), dense.at(detail::getIndex(sid).value));

            *removed_element = invalid_entity;
            dense.pop_back();
        }

        //: size and extent
        [[nodiscard]] std::size_t size() const { return dense.size(); }
        [[nodiscard]] std::size_t extent() const { return sparse.size(); }

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
    };
}