//* ecs
//      entity component system implementation
//      heavily influenced by the design @skypjack [entt](https://github.com/skypjack/entt),
//      along with their helpful blog posts, [ecs back and forth](https://skypjack.github.io/2019-02-14-ecs-baf-part-1)
//      also would like to highlight @kgorking beautiful implementation of an [ecs](https://github.com/kgorking/ecs)
//      and david colson's amazing [post](https://www.david-colson.com/2020/02/09/making-a-simple-ecs.html)
#pragma once

#include "std_types.h"
#include "type_name.h"
#include "log.h"
#include <deque>
#include <optional>

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
    using Index = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<detail::ID>, strong::Ordered, strong::Hashable>;
    using Version = strong::Type<ui16, decltype([]{}), strong::Regular, strong::ConvertibleTo<detail::ID>, strong::Ordered, strong::Arithmetic>;

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

    namespace detail
    {
        //* base component pool
        struct ComponentPoolBase {
            //: default constructor, no copy or move
            ComponentPoolBase() = default;
            ComponentPoolBase(const ComponentPoolBase&) = delete;
            ComponentPoolBase& operator=(const ComponentPoolBase&) = delete;
            ComponentPoolBase(ComponentPoolBase&&) = delete;
            ComponentPoolBase& operator=(ComponentPoolBase&&) = delete;
            virtual ~ComponentPoolBase() = default;

            //: alias for sparse set
            using SparseID = detail::ID;

            //: main array pair, sparse and dense
            std::unordered_map<Index, std::array<SparseID, engine_config.ecs_page_size()>> sparse;
            std::vector<Index> dense;

            //: get sparse
            //      gets the entity index and sees if it is included in the sparse array
            [[nodiscard]] const SparseID* sparse_at(const EntityID entity) const {
                const auto pos = index(entity).value;
                const auto page = pos / engine_config.ecs_page_size();
                return sparse.contains(page) ? &(sparse.at(page).at(pos % engine_config.ecs_page_size())) : nullptr;
            }
            [[nodiscard]] SparseID* sparse_at(const EntityID entity) {
                const auto pos = index(entity).value;
                const auto page = pos / engine_config.ecs_page_size();
                return sparse.contains(page) ? &(sparse.at(page).at(pos % engine_config.ecs_page_size())) : nullptr;
            }

            //: is valid
            //      checks if an sparse id handle points to something and has a propper version
            [[nodiscard]] constexpr bool valid(const SparseID* sid, const Version v) const {
                return sid != nullptr and *sid != invalid_id and version(*sid) == v;
            }

            //: contains entity
            //      checks if the entity is included in the sparse array, also verifying the version
            [[nodiscard]] bool contains(const EntityID entity) const {
                return valid(sparse_at(entity), version(entity));
            }
            
            //: remove is a constexpr virtual functions that is overriden by the derived classes
            constexpr virtual void remove(const EntityID entity) = 0;

            //: size and extent
            [[nodiscard]] std::size_t size() const { return dense.size(); }
            [[nodiscard]] std::size_t extent() const { return sparse.size() * engine_config.ecs_page_size(); }
        };
    }

    //* typed component pool
    template <typename T>
    struct ComponentPool : detail::ComponentPoolBase {
        //: data
        std::vector<T> data;

        //: add
        //      adds an entity to the sparse array, if there is an entity with a lower version it is updated
        //      if the entity has the same or higher version, an error is thrown
        constexpr void add(const EntityID entity, T&& value) {
            const auto pos = index(entity).value;
            const auto page = pos / engine_config.ecs_page_size();

            if (not sparse.contains(page))
                sparse[page].fill(invalid_id);

            auto& element = *sparse_at(pos);
            if (element == invalid_id) {
                element = id(dense.size(), version(entity));
                data.emplace_back(std::move(value));
                dense.emplace_back(index(entity));
            } else if (version(entity) > version(element)) {
                element = id(index(element), version(entity));
                data.at(index(element).value) = std::move(value);
                dense.at(index(element).value) = index(entity);
            } else {
                log::error("entity {} with version {} already exists in sparse set", entity.value, version(entity).value);
            }
        }

        //: get
        //      returns a pointer to the entity value from the dense array if it exists, if not it returns nullptr
        [[nodiscard]] constexpr auto get(const EntityID entity) {
            const auto sid = sparse_at(entity);
            return valid(sid, version(entity)) ? std::optional<const std::reference_wrapper<T>>(data.at(index(*sid).value)) : std::nullopt;
        }

        //: remove
        //      removes an entity if it exists, otherwise it does nothing
        //      it swaps the last element with the removed element from both the sparse and dense arrays
        //      this function uses find if to find the last entity, however, it might be faster to use a dedicated extra array that holds
        //      the references to the entities from each spot of the dense array
        constexpr void remove(const EntityID entity) override {
            const auto sid = sparse_at(entity);
            if (not valid(sid, version(entity))) return;

            SparseID* last_sparse = sparse_at(dense.back().value);
            auto removed_element = sparse_at(index(entity).value);

            std::swap(*last_sparse, *removed_element);
            std::swap(dense.back(), dense.at(index(*last_sparse).value));
            std::swap(data.back(), data.at(index(*last_sparse).value));

            *removed_element = invalid_id;
            data.pop_back();
            dense.pop_back();
        }

        //: clear
        constexpr void clear() {
            log::info("clearing {}", type_name<T>());
            sparse.clear();
            data.clear();
        }
    };

    //---

    //* scene
    //      a scene holds a number of component pools, each with its entities
    //      the scene is responsible for adding, updating and removing the entities
    struct Scene {
        //* component pool
        //      searchs the hash map for the specified component type
        //      if none is found, create it. this operation is thread safe

        //: hash map of component pools, using the component type as a key
        std::unordered_map<TypeHash, std::unique_ptr<detail::ComponentPoolBase>> component_pools;

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
        //      an entity is just a handle made from an index and a version
        //      the free entities list contains first any entity that has been deleted and can be reused and last a value of the next entity to be created
        //      reusing previous entity ids will always come first, the version parameter is used to distinguis between entities that have been reused
        std::deque<EntityID> free_entities = {ecs::id(0, 0)};

        //: add entity
        template <typename ... C>
        constexpr const EntityID add(C&& ... components) {
            const auto entity = free_entities.front();
            if (free_entities.size() > 1) free_entities.pop_front();
            else free_entities.back() = entity.value + 1;
            (cpool<C>().add(entity, std::forward<C>(components)), ...);
            return entity;
        }

        //: get entity component
        template <typename C>
        [[nodiscard]] constexpr auto get(const EntityID entity) {
            return cpool<C>().get(entity);
        }

        //: remove entity
        void remove(const EntityID entity) {
            [&] { for (auto &[key, pool] : component_pools) pool->remove(entity); }();
            free_entities.push_front(id(index(entity), version(entity) + Version(1)));
        }

        //- number of entities
    };

    //* view
    //      a view is an iterator range over some entities in a scene
    //      the view can be created over entities with one or more components, and will iterate over the entities that have all of this components
    //      there is a specialization for only one component that should be faster since it doesn't have to calculate the intersection
    //      iterating over a view should be relatively performant since it uses the dense array of the component pool
    template <typename ... C> requires (sizeof...(C) > 0)
    struct View {
        //: scene pointer
        Scene* scene;

        //: component pools and intersection of entity indices
        std::array<detail::ComponentPoolBase*, sizeof...(C)> pools;
        std::vector<Index> indices;

        //: constructor
        constexpr View(Scene& s) : scene(&s) {
            int i = 0; ([&]{ pools.at(i++) = &scene->cpool<C>(); }(), ...);
        }

        //: intersection of all the pools (entities that contain all components specified)
        [[nodiscard]] constexpr auto intersection() noexcept {
            auto last = pools.front()->dense;
            std::sort(last.begin(), last.end());
            decltype(last) current;
            for (auto &pool : pools) {
                auto dense = pool->dense;
                std::sort(dense.begin(), dense.end());
                std::set_intersection(last.begin(), last.end(), dense.begin(), dense.end(), std::back_inserter(current));
                std::swap(last, current);
                current.clear();
            }
            return last;
        }

        //: iterators
        //      iterates over all entities that have all the specified components
        //      for (auto [entity, data1, data2] : view()) ... (note, no & is needed, they are already references)
        [[nodiscard]] constexpr auto operator()() noexcept {
            indices = intersection();
            return rv::zip(rv::const_(indices), [&]{
                auto &pool = scene->cpool<C>();
                return rv::zip(pool.dense, pool.data) | rv::filter([&](auto a) {
                    return std::count(indices.begin(), indices.end(), a.first);
                }) | rv::values;
            }()...);
        }
        //      you can iterate over just the data using:
        //      for (auto [data1, data2] : view.data())
        [[nodiscard]] constexpr auto data() noexcept {
            indices = intersection();
            return rv::zip([&]{
                auto &pool = scene->cpool<C>();
                return rv::zip(pool.dense, pool.data) | rv::filter([&](auto a) {
                    return std::count(indices.begin(), indices.end(), a.first);
                }) | rv::values;
            }()...);
        }
    };
    //: specialization for only one component (should be faster)
    template <typename C>
    struct View<C> {
        //: scene pointer
        Scene* scene;

        //: constructor
        constexpr View(Scene& s) : scene(&s) {}

        //: iterators
        //      iterates over all entities that have the specified component
        //      for (auto [entity, data] : view()) ... (note, no & is needed, they are already references)
        [[nodiscard]] constexpr auto operator()() noexcept { return rv::zip(rv::const_(scene->cpool<C>().dense), scene->cpool<C>().data); }
        //      to iterate over just the data use it like:
        //      for (auto &data) : view.data()) ...
        [[nodiscard]] constexpr auto& data() noexcept { return scene->cpool<C>().data; }
    };
}